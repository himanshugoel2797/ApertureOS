#ifndef _INTEL_HD_AUDIO_DRIVER_H_
#define _INTEL_HD_AUDIO_DRIVER_H_

#include "types.h"
#include "drivers.h"
#include "managers.h"

uint32_t
IHDA_Initialize(void);

void 
IHDA_Write(uint32_t offset, 
           uint32_t val);

uint32_t 
IHDA_Read(uint32_t offset);

void
IHDA_Reset(void);

void
IHDA_DetectCodecs(void);

void
IHDA_SetupCORB(void);

void
IHDA_SetupRIRB(void);

#endif