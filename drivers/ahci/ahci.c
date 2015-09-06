#include "ahci.h"
#include "priv_ahci.h"
#include "managers.h"

uint32_t ahci_memory_base = 0;
HBA_MEM *hba_mem;
HBA_FIS *hba_fis;
HBA_CMD_HEADER *hba_cmd_header;
HBA_PORT *port;


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

            hba_mem = (HBA_MEM*)ahci_memory_base;

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

                        if( hba_mem->ports[i].clbu == hba_mem->ports[i].fbu &&
                                hba_mem->ports[i].clb - hba_mem->ports[i].fb > 0xFFFFFF)
                        {

                            //setup the FIS and CMD LIST tables and map them into kernel memory
                            hba_fis = virtMemMan_FindEmptyAddress(KB(8), MEM_KERNEL);
                            if(hba_fis == NULL)break;
                            if(virtMemMan_Map(hba_fis, hba_mem->ports[i].fb | ((uint64_t)hba_mem->ports[i].fbu) << 32, KB(4), MEM_TYPE_WB, MEM_READ | MEM_WRITE, MEM_KERNEL) < 0)break;

                            hba_fis = (uint32_t)hba_fis + (hba_mem->ports[i].fb % KB(4));
                            hba_cmd_header = ((uint32_t)hba_fis + (hba_mem->ports[i].clb - hba_mem->ports[i].fb));

                        }
                        else
                        {
                            //setup the FIS and CMD LIST tables and map them into kernel memory
                            hba_fis = virtMemMan_FindEmptyAddress(KB(4), MEM_KERNEL);
                            if(hba_fis == NULL)break;
                            if(virtMemMan_Map(hba_fis, hba_mem->ports[i].fb | ((uint64_t)hba_mem->ports[i].fbu) << 32, KB(4), MEM_TYPE_WB, MEM_READ | MEM_WRITE, MEM_KERNEL) < 0)break;

                            hba_cmd_header = virtMemMan_FindEmptyAddress(KB(8), MEM_KERNEL);
                            if(hba_cmd_header == NULL)break;
                            if(virtMemMan_Map(hba_cmd_header, hba_mem->ports[i].clb | ((uint64_t)hba_mem->ports[i].clbu) << 32, KB(8), MEM_TYPE_WB, MEM_READ | MEM_WRITE, MEM_KERNEL) < 0)break;
                            hba_cmd_header = (uint32_t)hba_cmd_header + (hba_mem->ports[i].clb % KB(4));
                            hba_fis = (uint32_t)hba_fis + (hba_mem->ports[i].fb % KB(4));
                        }

                        //Adjust both addresses so they compensate for alignment changes

                        COM_WriteStr("hba %x\r\n", hba_fis);

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
    port->is = (uint32_t)-1;       // Clear pending interrupt bits
    int spin = 0; // Spin lock timeout counter
    int slot = find_cmdslot(port);
    if (slot == -1)
        return FALSE;

    HBA_CMD_HEADER *cmdheader = hba_cmd_header;//[slot];
    cmdheader += slot;
    cmdheader->cfl = sizeof(FIS_REG_H2D)/sizeof(uint32_t); // Command FIS size
    cmdheader->w = 0;       // Read from device
    cmdheader->prdtl = (uint16_t)((count-1)>>4) + 1;    // PRDT entries count

    COM_WriteStr("CTBA %x\r\n", cmdheader->ctba);
    HBA_CMD_TBL *cmdtbl = (HBA_CMD_TBL*)(cmdheader->ctba);
    memset(cmdtbl, 0, sizeof(HBA_CMD_TBL) +
           (cmdheader->prdtl-1)*sizeof(HBA_PRDT_ENTRY));

    int i = 0;


    // 8K bytes (16 sectors) per PRDT
    for (; i<cmdheader->prdtl-1; i++)
    {
        cmdtbl->prdt_entry[i].dba = (uint32_t)buf;
        cmdtbl->prdt_entry[i].dbc = 8*1024; // 8K bytes
        cmdtbl->prdt_entry[i].i = 1;
        buf += 4*1024;  // 4K words
        count -= 16;    // 16 sectors
    }
    // Last entry
    cmdtbl->prdt_entry[i].dba = (uint32_t)buf;
    cmdtbl->prdt_entry[i].dbc = count<<9;   // 512 bytes per sector
    cmdtbl->prdt_entry[i].i = 1;

    // Setup command
    FIS_REG_H2D *cmdfis = (FIS_REG_H2D*)(&cmdtbl->cfis);

    cmdfis->fis_type = FIS_TYPE_REG_H2D;
    cmdfis->c = 1;  // Command
    cmdfis->command = 0x25;

    cmdfis->lba0 = (uint8_t)startl;
    cmdfis->lba1 = (uint8_t)(startl>>8);
    cmdfis->lba2 = (uint8_t)(startl>>16);
    cmdfis->device = 1<<6;  // LBA mode

    cmdfis->lba3 = (uint8_t)(startl>>24);
    cmdfis->lba4 = (uint8_t)starth;
    cmdfis->lba5 = (uint8_t)(starth>>8);

    cmdfis->countl = count & 0xFF;
    cmdfis->counth = (count >> 8) & 0xFF;

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
        /*
        if (port->is & HBA_PxIS_TFES)   // Task file error
        {
            COM_WriteStr("Read disk error\r\n");
            return FALSE;
        }*/
    }

    //COM_WriteStr("TESt %x\r\n", cmdheader);
    /*
    // Check again
    if (port->is & HBA_PxIS_TFES)
    {
        COM_WriteStr("Read disk error\r\n");
        return FALSE;
    }*/

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