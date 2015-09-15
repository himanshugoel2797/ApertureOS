#include "rtl8139.h"
#include "drivers.h"
#include "utils/native.h"

uint32_t rtl8139_index = 0;
uint32_t rtl8139_io_bar = 0;

bool
RTL8139_Detect(void)
{
	int i;

	//Search PCI devices for devices of this family
	for (i = 0; i < pci_deviceCount; i++)
	{
		//Official IDs
		if(devices[i].vendorID == 0x10EC)
		{
			switch(devices[i].deviceID)
			{
				case 0x8139:
				case 0x8339:
					return TRUE;
			}
		}

		if(devices[i].vendorID == 0x1904)
		{
			switch(devices[i].deviceID)
			{
				case 0x8139:
					return TRUE;
			}
		}

		//Unofficial IDs
		if(devices[i].vendorID == 0x14EA && devices[i].deviceID == 0xAB06)
		{
			return TRUE;
		}

		if(devices[i].vendorID == 0x1186 && devices[i].deviceID == 0x1300)
		{
			return TRUE;
		}

		if(devices[i].vendorID == 0x1065 && devices[i].deviceID == 0x8139)
		{
			return TRUE;
		}

		if(devices[i].vendorID == 0x10BD && devices[i].deviceID == 0x8139)
		{
			return TRUE;
		}

		if(devices[i].vendorID == 0x1113 && devices[i].deviceID == 0x1211)
		{
			return TRUE;
		}		

		if(devices[i].vendorID == 0x1259 && devices[i].deviceID == 0x2503)
		{
			return TRUE;
		}
	}

	return FALSE;	//The device wasn't found
}

void
RTL8139_Outl(uint32_t offset, uint32_t val)
{
	outl(rtl8139_io_bar + offset, val);
}

void
RTL8139_Outb(uint32_t offset, uint8_t val)
{
	outb(rtl8139_io_bar + offset, val);
}

uint32_t
RTL8139_Initialize(uint32_t deviceIndex)
{
	//Enable bus master
	pci_setCommand(deviceIndex, PCI_BUS_MASTER_CMD);
	rtl8139_index = deviceIndex;
	rtl8139_io_bar = devices[rtl8139_index].bars[0];	//Maybe use MMIO instead
}

uint32_t
RTL8139_Reset(void)
{

}