#ifndef _PHYS_MEM_MANAGER_H_
#define _PHYS_MEM_MANAGER_H_

#include "types.h"

void physMemMan_Setup();
void* physMemMan_Alloc(uint64_t size);
void physMemMan_Free(void *ptr);

#endif /* end of include guard: _PHYS_MEM_MANAGER_H_ */
