#ifndef _GRAPHICS_MAIN_H_
#define _GRAPHICS_MAIN_H_

#include <stddef.h>
#include <stdint.h>
#include "multiboot.h"

int Graphics_Initialize(VbeInfoBlock *infoBlock, ModeInfoBlock *modeInfo, VESA_PM_INFO *pmode);

#endif /* end of include guard: _GRAPHICS_MAIN_H_ */
