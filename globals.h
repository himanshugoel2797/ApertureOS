#ifndef _GLOBALS_H_
#define _GLOBALS_H_

#include <stdint.h>
#include <stddef.h>
#include "multiboot.h"

VbeInfoBlock *global_vbe_info;
ModeInfoBlock *global_mode_info;
multiboot_info_t *global_multiboot_info;
multiboot_memory_map_t *global_memory_map;
size_t global_memory_map_size;

#endif /* end of include guard: _GLOBALS_H_ */
