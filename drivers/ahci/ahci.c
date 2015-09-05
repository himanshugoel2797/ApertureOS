#include "ahci.h"
#include "priv_ahci.h"
#include "managers.h"

uint32_t ahci_memory_base = 0;
HBA_MEM *hba_mem;
HBA_FIS *hba_fis;
HBA_CMD_HEADER *hba_cmd_header;


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
            for(int i = 0; i < 32; i++)
            {
                if(((hba_mem->pi >> i) & 1) == 1)
                {
                    if(AHCI_CheckDeviceType(&hba_mem->ports[i]) == AHCI_DEV_SATA)
                    {
                        COM_WriteStr("Using port #%d\r\n", i);

                        //setup the FIS and CMD LIST tables and map them into kernel memory
                        hba_fis = virtMemMan_FindEmptyAddress(KB(4), MEM_KERNEL);
                        if(hba_fis == NULL)break;
                        if(virtMemMan_Map(hba_fis, hba_mem->ports[i].fb | ((uint64_t)hba_mem->ports[i].fbu) << 32, KB(4), MEM_TYPE_WB, MEM_READ | MEM_WRITE, MEM_KERNEL) < 0)break;

                        hba_cmd_header = virtMemMan_FindEmptyAddress(MB(1), MEM_KERNEL);
                        if(hba_cmd_header == NULL)break;
                        if(virtMemMan_Map(hba_cmd_header, hba_mem->ports[i].clb | ((uint64_t)hba_mem->ports[i].clbu) << 32, MB(1), MEM_TYPE_WB, MEM_READ | MEM_WRITE, MEM_KERNEL) < 0)break;

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