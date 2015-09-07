#include "ahci.h"
#include "priv_ahci.h"
#include "managers.h"
#include "kmalloc.h"

uint32_t ahci_memory_base = 0;
HBA_MEM *hba_mem;
HBA_PORT *port;

uint32_t AHCI_BASE;

void port_rebase(HBA_PORT *port, int portno);
void stop_cmd(HBA_PORT *port);
void start_cmd(HBA_PORT *port);


uint8_t AHCI_Initialize()
{
    //Search for AHCI compatible controllers in the PCI device list
    for(int i = 0; i < pci_deviceCount; i++)
    {
        if(devices[i].classCode == PCI_MASS_STORAGE_DEVICE_CLASS && devices[i].subClassCode == 0x06)
        {
            //Found an AHCI compatible device!
            COM_WriteStr("Found an AHCI controller!\r\n");
            if(devices[i].bar_count < 6)break;

            //Check to see if the base address has already been mapped due to the top half mapping scheme
            if(devices[i].bars[5] < MEMIO_TOP_BASE)
            {
                //Map the ahci memory base address into kernel memory
                ahci_memory_base = (uint32_t)virtMemMan_FindEmptyAddress(KB(8), MEM_KERNEL);
                if(ahci_memory_base == NULL)break;

                if(virtMemMan_Map(ahci_memory_base, devices[i].bars[5], KB(8), MEM_TYPE_UC, MEM_WRITE | MEM_READ, MEM_KERNEL) < 0)break;
            }
            else
            {
                ahci_memory_base = VIRTUALIZE_HIGHER_MEM_OFFSET(devices[i].bars[5]);
            }

            //Enable PCI busmastering for this device
            pci_setCommand(i, PCI_BUS_MASTER_CMD);

            hba_mem = (HBA_MEM*)ahci_memory_base;

            //Obtain ownership of the controller if ownershp handoff is supported
            if(hba_mem->cap2 & 1){
                hba_mem->bohc |= 2;
                while((hba_mem->bohc & 1) || !(hba_mem->bohc & 2))ThreadMan_Yield();
            }

            //Software Reset the controller
            hba_mem->ghc |= (1 << 31);  //Enable AHCI
            hba_mem->ghc |= 1;  //Reset
            while(hba_mem->ghc & 1)ThreadMan_Yield();
            hba_mem->ghc |= (1 << 31);  //Re-enable AHCI

            COM_WriteStr("AHCI Version: %d%d.%d%d\r\n", (hba_mem->vs >> 24) & 0xFF, (hba_mem->vs >> 16) & 0xFF, (hba_mem->vs >> 8) & 0xFF, hba_mem->vs & 0xFF);
            COM_WriteStr("Ports: %b\r\n", hba_mem->pi);

            //Find the index of the first port that is an ATA Disk
            for(uint32_t i = 0; i < 32; i++)
            {
                if(((hba_mem->pi >> i) & 1) == 1)
                {
                    if(AHCI_CheckDeviceType(&hba_mem->ports[i]) == AHCI_DEV_SATA)
                    {
                        port = &hba_mem->ports[i];
                        COM_WriteStr("Using port #%d\r\n", i);

                        AHCI_BASE = bootstrap_malloc(KB(512));
                        AHCI_BASE += (AHCI_BASE % 1000);

                        port_rebase(port, i);

                        return 0;
                    }
                }
            }

        }
    }
    return -1;
}

uint8_t AHCI_CheckDeviceType(HBA_PORT *port)
{
    uint32_t ssts = port->ssts;

    uint8_t ipm = (ssts >> 8) & 0x0F;
    uint8_t det = (ssts & 0x0F);

    if(ipm != HBA_PORT_IPM_ACTIVE | det != HBA_PORT_DET_PRESENT)return HBA_NO_DEVICE;


    switch (port->sig)
    {
    case SATA_SIG_ATAPI:
        return AHCI_DEV_SATAPI;
    case SATA_SIG_SEMB:
        return AHCI_DEV_SEMB;
    case SATA_SIG_PM:
        return AHCI_DEV_PM;
    default:
        return AHCI_DEV_SATA;
    }

}

bool AHCI_Read(uint32_t startl, uint32_t starth, uint32_t count, uint16_t *buf)
{
    port->is = ~0;       // Clear pending interrupt bits
    int spin = 0; // Spin lock timeout counter
    int slot = find_cmdslot(port);

    if (slot == -1)
        return FALSE;

    HBA_CMD_HEADER *cmdheader = (HBA_CMD_HEADER*)port->clb;//[slot];
    cmdheader = &cmdheader[slot];

    cmdheader->cfl = sizeof(FIS_REG_H2D)/sizeof(uint32_t); // Command FIS size
    cmdheader->w = 0;       // Read from device
    cmdheader->prdtl = (uint16_t)((count-1)>>4) + 1;    // PRDT entries count

    HBA_CMD_TBL *cmd_tbl = (HBA_CMD_TBL*)cmdheader->ctba;
    memset(cmd_tbl, 0, sizeof(HBA_CMD_TBL) + sizeof(HBA_PRDT_ENTRY) * (count - 1));

    FIS_REG_H2D *fis = (FIS_REG_H2D*)cmd_tbl->cfis;
    COM_WriteStr("Addr: %x\r\n", fis);

    fis->fis_type = FIS_TYPE_REG_H2D;
    fis->pmport = 0;
    fis->rsv0 = 0;
    fis->c = 1;
    fis->command = 0x24;

    fis->lba0 = startl;
    fis->lba1 = startl >> 8;
    fis->lba2 = startl >> 16;
    fis->lba3 = startl >> 24;
    fis->lba4 = starth;
    fis->lba5 = starth >> 8;

    fis->device = 1 << 6;
    fis->countl = count;
    fis->counth = count >> 8;

    for(int i = 0; i < cmdheader->prdtl; i++)
    {
        cmd_tbl->prdt_entry[i].dba = (uint32_t)buf;
        cmd_tbl->prdt_entry[i].dbau = 0;
        cmd_tbl->prdt_entry[i].rsv0 = 0;
        cmd_tbl->prdt_entry[i].dbc = KB(8) - 1;
        cmd_tbl->prdt_entry[i].rsv1 = 0;
        cmd_tbl->prdt_entry[i].i = 0;

        buf = ((uint32_t)buf) + KB(8);
    }

    // The below loop waits until the port is no longer busy before issuing a new command
    while ((port->tfd & (ATA_DEV_BUSY | ATA_DEV_DRQ)) && spin < 1000000)
    {
        spin++;
        COM_WriteStr("%d\r\n", spin);
    }
    if (spin == 1000000)
    {
        COM_WriteStr("Port is hung\r\n");
        return FALSE;
    }

    port->ci = 1<<slot; // Issue command

    // Wait for completion
    while (1)
    {
        // In some longer duration reads, it may be helpful to spin on the DPS bit
        // in the PxIS port field as well (1 << 5)
        if ((port->ci & (1<<slot)) == 0)
            break;

        if (port->is & (1<<30))   // Task file error
        {
            COM_WriteStr("Read disk error\r\n");
            return FALSE;
        }
    }

    //COM_WriteStr("TESt %x\r\n", cmdheader);
    // Check again
    if (port->is & (1<<30))
    {
        COM_WriteStr("Read disk error\r\n");
        return FALSE;
    }

    return TRUE;
}

// Find a free command list slot
int find_cmdslot(HBA_PORT *port)
{
    // If not set in SACT and CI, the slot is free
    uint32_t slots = (port->sact | port->ci);
    for (int i=0; i<32; i++)
    {
        if ((slots&1) == 0)
            return i;
        slots >>= 1;
    }
    COM_WriteStr("Cannot find free command list entry\r\n");
    return -1;
}

void port_rebase(HBA_PORT *port, int portno)
{
    stop_cmd(port); // Stop command engine
 
    // Command list offset: 1K*portno
    // Command list entry size = 32
    // Command list entry maxim count = 32
    // Command list maxim size = 32*32 = 1K per port
    port->clb = AHCI_BASE + (portno<<10);
    port->clbu = 0;
    memset((void*)(port->clb), 0, 1024);
 

    // FIS offset: 32K+256*portno
    // FIS entry size = 256 bytes per port
    port->fb = AHCI_BASE + (32<<10) + (portno<<8);
    port->fbu = 0;
    memset((void*)(port->fb), 0, 256);
 
    // Command table offset: 40K + 8K*portno
    // Command table size = 256*32 = 8K per port
    HBA_CMD_HEADER *cmdheader = (HBA_CMD_HEADER*)(port->clb);
    for (int i=0; i<32; i++)
    {
        cmdheader[i].prdtl = 8; // 8 prdt entries per command table
                    // 256 bytes per command table, 64+16+48+16*8
        // Command table offset: 40K + 8K*portno + cmdheader_index*256
        cmdheader[i].ctba = AHCI_BASE + (40<<10) + (portno<<13) + (i<<8);
        COM_WriteStr("%d:%x\r\n", i, cmdheader[i].ctba);
        cmdheader[i].ctbau = 0;
        memset((void*)cmdheader[i].ctba, 0, 256);
    }
 
    start_cmd(port);    // Start command engine
    uint32_t c = port->cmd; //Flush the cmd buffer
    c = c;
}
 
// Start command engine
void start_cmd(HBA_PORT *port)
{
    // Wait until CR (bit15) is cleared
    while (port->cmd & (1<<15));
 
    // Set FRE (bit4) and ST (bit0)
    port->cmd |= 1<<4;
    port->cmd |= 1;
    //COM_WriteStr("%b\r\n", port->cmd);
}
 
// Stop command engine
void stop_cmd(HBA_PORT *port)
{
    // Clear ST (bit0)
    port->cmd &= ~1;

    // Clear FRE (bit4);
    port->cmd &= ~(1<<4);
 
    // Wait until FR (bit14), CR (bit15) are cleared
    while(1)
    {
        if (port->cmd & (1<<14))
            continue;
        if (port->cmd & (1<<15))
            continue;
        break;
    }
}