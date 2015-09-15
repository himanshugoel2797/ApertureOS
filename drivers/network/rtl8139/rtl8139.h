#ifndef _NETWORK_RTL_8139_DRIV_H_
#define _NETWORK_RTL_8139_DRIV_H_

#include "types.h"
#include "../network.h"

bool
RTL8139_Detect(void);

uint32_t
RTL8139_Initialize(uint32_t deviceIndex);

uint32_t
RTL8139_Reset(void);

#endif