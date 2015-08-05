#ifndef _GLOBALS_H_
#define _GLOBALS_H_

#include <stdint.h>
#include <stddef.h>
#include "multiboot.h"

VESA_PM_INFO *global_pm_info;
VbeInfoBlock *global_vbe_info;
ModeInfoBlock *global_mode_info;
multiboot_info_t *global_multiboot_info;


#endif /* end of include guard: _GLOBALS_H_ */
