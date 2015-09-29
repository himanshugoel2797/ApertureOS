#include "pci.h"
#include "priv_pci.h"

#include "utils/native.h"
#include "acpi_tables/mcfg.h"

#include "managers.h"

MCFG_Entry *mcfg_tables = NULL;
uint32_t mcfg_entry_count = 0;

uint32_t
pci_readDWord(
    uint32_t bus,
    uint32_t device,
    uint32_t function,
    uint32_t offset
)
{
    outl(PCI_ADDR, 0x80000000 | bus << 16 | device << 11 | function <<  8 | (offset & 0xfc));
    return inl(PCI_DATA);
}

void
pci_writeDWord(
    uint32_t bus,
    uint32_t device,
    uint32_t function,
    uint32_t offset,
    uint32_t val
)
{
    outl(PCI_ADDR, 0x80000000 | bus << 16 | device << 11 | function <<  8 | (offset & 0xfc));
    outl(PCI_DATA, val);
}

void
pci_Initialize(void)
{
    COM_WriteStr("\r\nEnumerating PCI devices:\r\n");

    memset(pci_devices, 0, sizeof(PCI_DeviceFuncs) * MAX_DEVICE_COUNT);
    pci_deviceCount = 0;

    //Enumerate PCI pci_devices
    for(int bus = 0; bus < 256; bus++)
        {
            for(int device = 0; device < 32; device++)
                {
                    uint32_t vid = pci_readDWord(bus, device, 0, 0);
                    if( (vid >> 16) != 0xFFFF)
                        {
                            int headerType = pci_readDWord(bus, device, 0, 0x0C);
                            int functionCount = 1;
                            if( (headerType >> 23) & 1) functionCount = 8;

                            for(int f = 0; f < functionCount; f++)
                                {
                                    char *base, *sub, *prog;
                                    char *vendor_short, *vendor_long;
                                    char *chip_name, *chip_desc;

                                    if(pci_readDWord(bus, device, f, 0) >> 16 != 0xFFFF)
                                        {

                                            pci_devices[pci_deviceCount].classCode = pci_readDWord(bus, device, f, 8) >> 24;
                                            pci_devices[pci_deviceCount].subClassCode = pci_readDWord(bus, device, f, 8) >> 16;
                                            pci_devices[pci_deviceCount].progIf = pci_readDWord(bus, device, f, 8) >> 8;
                                            pci_devices[pci_deviceCount].bus = bus;
                                            pci_devices[pci_deviceCount].device = device;
                                            pci_devices[pci_deviceCount].function = f;
                                            pci_devices[pci_deviceCount].headerType = (pci_readDWord(bus, device, f, 0x0C) >> 16) & 0xFF;

                                            pci_devices[pci_deviceCount].deviceID = pci_readDWord(bus, device, f, 0) >> 16;
                                            pci_devices[pci_deviceCount].vendorID = pci_readDWord(bus, device, f, 0);

                                            switch(pci_devices[pci_deviceCount].headerType)
                                                {
                                                case 0:
                                                    pci_devices[pci_deviceCount].bar_count = 6;
                                                    break;
                                                case 1:
                                                    pci_devices[pci_deviceCount].bar_count = 2;
                                                    break;
                                                }

                                            for(uint8_t bar_index = 0; bar_index < pci_devices[pci_deviceCount].bar_count; bar_index++)
                                                {
                                                    pci_devices[pci_deviceCount].bars[bar_index] = pci_readDWord(bus, device, f, 0x10 + (bar_index * 4));
                                                }


                                            pci_GetPCIClass(pci_readDWord(bus, device, f, 8),
                                                            &base, &sub, &prog);

                                            pci_GetPCIDevice(pci_devices[pci_deviceCount].vendorID,
                                                             pci_devices[pci_deviceCount].deviceID,
                                                             &chip_name,
                                                             &chip_desc);

                                            pci_GetPCIVendor(pci_devices[pci_deviceCount].vendorID,
                                                             &vendor_short,
                                                             &vendor_long);


                                            COM_WriteStr("\tFound %s %s %s(%d.%d.%d), %s from %s at %d:%d:%d\r\n",
                                                         sub, prog, base,
                                                         pci_devices[pci_deviceCount - 1].classCode,
                                                         pci_devices[pci_deviceCount - 1].subClassCode,
                                                         pci_devices[pci_deviceCount - 1].progIf,
                                                         chip_name,
                                                         vendor_short,
                                                         bus, device, f);

                                            pci_deviceCount++;
                                        }
                                }
                        }
                }
        }

    //Initialize PCI Express info
    MCFG* mcfg = ACPITables_FindTable(MCFG_SIG, 0);

    if(mcfg == NULL)
        {
            COM_WriteStr("\r\nFailed to find MCFG table, driver will only support PCI pci_devices!\r\n");
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

void
pci_setCommand(uint32_t device_index,
               uint16_t value)
{
    uint32_t reg = pci_readDWord(
                       pci_devices[device_index].bus,
                       pci_devices[device_index].device,
                       pci_devices[device_index].function,
                       0x04);

    //reg &= 0xFFFF0000;
    reg |= value;
    pci_writeDWord(
        pci_devices[device_index].bus,
        pci_devices[device_index].device,
        pci_devices[device_index].function,
        0x04,
        reg);
}