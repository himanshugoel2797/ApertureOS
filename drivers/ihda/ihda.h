#ifndef _INTEL_HD_AUDIO_DRIVER_H_
#define _INTEL_HD_AUDIO_DRIVER_H_

#include "types.h"
#include "drivers.h"
#include "managers.h"

uint32_t
IHDA_Initialize(void);

void
IHDA_Reset(void);

void
IHDA_DetectCodecs(void);

void
IHDA_SetupCORB(void);

void
IHDA_SetupRIRB(void);

void
IHDA_WriteVerb(uint32_t verb);

uint64_t
IHDA_ReadResponse(void);

#endif