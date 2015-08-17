#ifndef _PCI_DRIVER_H_
#define _PCI_DRIVER_H_

#include "types.h"

void pci_GetPCIClass (
        long classcode,
        char ** base,
        char ** sub,
        char ** prog
        );

void pci_Initialize();

#endif /* end of include guard: _PCI_DRIVER_H_ */
