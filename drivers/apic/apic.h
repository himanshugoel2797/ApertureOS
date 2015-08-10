#ifndef _APIC_DRIVER_H_
#define _APIC_DRIVER_H_

#include <stddef.h>
#include <stdint.h>

uint8_t APIC_Initialize();
void APIC_SetEnableMode(uint8_t enabled);

#endif /* end of include guard: _APIC_DRIVER_H_ */
