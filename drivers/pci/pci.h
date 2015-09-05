#ifndef _PCI_DRIVER_H_
#define _PCI_DRIVER_H_

#include "types.h"

#define MAX_DEVICE_COUNT 128
#define MAX_POSSIBLE_BARS 6

#define PCI_MASS_STORAGE_DEVICE_CLASS 0x1


typedef struct{
	uint8_t classCode;
	uint8_t subClassCode;
	uint8_t progIf;
	uint32_t bus;
	uint32_t device;
	uint32_t function;
	uint8_t bar_count;
	uint32_t bars[MAX_POSSIBLE_BARS];
	uint8_t headerType;
}PCI_DeviceFuncs;

PCI_DeviceFuncs devices[MAX_DEVICE_COUNT];
uint32_t pci_deviceCount;

void pci_GetPCIClass (
        long classcode,
        char ** base,
        char ** sub,
        char ** prog
        );

void pci_Initialize();

#endif /* end of include guard: _PCI_DRIVER_H_ */
