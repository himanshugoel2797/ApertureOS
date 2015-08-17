#include "pci.h"
#include "priv_pci.h"

#include "utils/native.h"
#include "acpi_tables/mcfg.h"

#include "managers.h"

MCFG_Entry *mcfg_tables = NULL;
uint32_t mcfg_entry_count = 0;

uint32_t pci_readDWord(
        uint32_t bus,
        uint32_t device,
        uint32_t function,
        uint32_t offset
        )
{
        outl(PCI_ADDR, 0x80000000 | bus << 16 | device << 11 | function <<  8 | (offset & 0xfc));
        return inl(PCI_DATA);
}

void pci_Initialize()
{
        //Enumerate PCI devices
        for(int bus = 0; bus < 256; bus++)
        {
                for(int device = 0; device < 32; device++)
                {
                        uint32_t vid = pci_readDWord(bus, device, 0, 0);
                        if(vid != 0xFFFFFFFF) COM_WriteStr("\r\nVendor %x\r\n", vid);
                }
        }

        //Initialize PCI Express info
        MCFG* mcfg = ACPITables_FindTable(MCFG_SIG, 0);

        if(mcfg == NULL) {
                COM_WriteStr("\r\nFailed to find MCFG table, driver will only support PCI devices!\r\n");
                return;
        }

        mcfg_entry_count = (mcfg->h.Length - sizeof(ACPISDTHeader) - 8)/sizeof(MCFG_Entry);
        mcfg_tables = bootstrap_malloc(mcfg_entry_count * sizeof(MCFG_Entry));
        memcpy(mcfg_tables, mcfg->entries, sizeof(MCFG_Entry) * mcfg_entry_count);

        for(int i = 0; i < mcfg_entry_count; i++)
        {
                COM_WriteStr("%x\r\n", mcfg_tables[i].baseAddr);
        }
}
