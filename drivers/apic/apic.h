#ifndef _APIC_DRIVER_H_
#define _APIC_DRIVER_H_

#include "types.h"
#include "io_apic/io_apic.h"

#define APIC_TIMER 0x320
#define APIC_THERMAL_SENSOR 0x330
#define APIC_PERF_MON 0x340
#define APIC_LINT0    0x350
#define APIC_LINT1    0x360

uint8_t APIC_LocalInitialize();
uint32_t APIC_Initialize();
void APIC_SetEnableMode(uint8_t enabled);
uint8_t APIC_GetID();
void APIC_SetEnableInterrupt(uint32_t interrupt, int enableMode);
void APIC_SetVector(uint32_t interrupt, uint8_t vector);
void APIC_SendEOI(uint8_t int_num);

#endif /* end of include guard: _APIC_DRIVER_H_ */
