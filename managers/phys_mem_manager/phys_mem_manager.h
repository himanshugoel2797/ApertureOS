#ifndef _PHYS_MEM_MANAGER_H_
#define _PHYS_MEM_MANAGER_H_

#include "types.h"

void physMemMan_Setup(void);
uint64_t physMemMan_Alloc(void);
void physMemMan_Free(uint64_t ptr);

#endif /* end of include guard: _PHYS_MEM_MANAGER_H_ */
