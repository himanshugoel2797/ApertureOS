#ifndef _HPET_DRIVER_H_
#define _HPET_DRIVER_H_

#include <stddef.h>
#include <stdint.h>

uint8_t HPET_Initialize();

uint64_t HPET_GetGlobalCounter();
void HPET_SetGlobalCounter(uint64_t val);

#endif /* end of include guard: _HPET_DRIVER_H_ */
