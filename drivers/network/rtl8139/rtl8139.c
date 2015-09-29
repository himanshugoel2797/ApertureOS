#include "rtl8139.h"
#include "drivers.h"
#include "managers.h"
#include "utils/native.h"

uint32_t rtl8139_index = 0;
uint8_t *rtl8139_mem_bar = 0;

bool
RTL8139_Detect(uint32_t i)
{
    //Official IDs
    if(pci_devices[i].vendorID == 0x10EC)
        {
            switch(pci_devices[i].deviceID)
                {
                case 0x8139:
                case 0x8339:
                    return TRUE;
                }
        }

    if(pci_devices[i].vendorID == 0x1904)
        {
            switch(pci_devices[i].deviceID)
                {
                case 0x8139:
                    return TRUE;
                }
        }

    //Unofficial IDs
    if(pci_devices[i].vendorID == 0x14EA && pci_devices[i].deviceID == 0xAB06)
        {
            return TRUE;
        }

    if(pci_devices[i].vendorID == 0x1186 && pci_devices[i].deviceID == 0x1300)
        {
            return TRUE;
        }

    if(pci_devices[i].vendorID == 0x1065 && pci_devices[i].deviceID == 0x8139)
        {
            return TRUE;
        }

    if(pci_devices[i].vendorID == 0x10BD && pci_devices[i].deviceID == 0x8139)
        {
            return TRUE;
        }

    if(pci_devices[i].vendorID == 0x1113 && pci_devices[i].deviceID == 0x1211)
        {
            return TRUE;
        }

    if(pci_devices[i].vendorID == 0x1259 && pci_devices[i].deviceID == 0x2503)
        {
            return TRUE;
        }

    return FALSE;	//The device wasn't found
}

void
RTL8139_Outl(uint32_t offset, uint32_t val)
{
    *(uint32_t*)(rtl8139_mem_bar + offset) = val;
}

void
RTL8139_Outb(uint32_t offset, uint8_t val)
{
    rtl8139_mem_bar[offset] = val;
}

uint32_t
RTL8139_Initialize(uint32_t deviceIndex)
{
    //Enable bus master
    pci_setCommand(deviceIndex, PCI_BUS_MASTER_CMD);
    rtl8139_index = deviceIndex;
    rtl8139_mem_bar = pci_devices[rtl8139_index].bars[1] & ~1;	//Use the MMIO interface

    COM_WriteStr("IO Base: %x", pci_devices[rtl8139_index].bars[0]);

    if(rtl8139_mem_bar < MEMIO_TOP_BASE)
        {
            //We'll have to map 256 bytes into RAM :|
        }
    else
        {
            rtl8139_mem_bar = VIRTUALIZE_HIGHER_MEM_OFFSET(rtl8139_mem_bar);
        }
}

uint32_t
RTL8139_Reset(void)
{

}