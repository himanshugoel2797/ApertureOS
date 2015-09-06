#ifndef _ACHI_DRIVER_H_
#define _ACHI_DRIVER_H_

#include "types.h"
#include "drivers.h"
#include "priv_ahci.h"

uint8_t AHCI_Initialize();
uint8_t AHCI_CheckDeviceType(HBA_PORT *port);
bool AHCI_Read(uint32_t startl, uint32_t starth, uint32_t count, uint16_t *buf);

#endif