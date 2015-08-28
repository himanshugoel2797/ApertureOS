#ifndef _PRIV_PHYS_MEM_MANAGER_H_
#define _PRIV_PHYS_MEM_MANAGER_H_

#include "types.h"

uint64_t memory_size;
uint64_t freePageCount;
uint64_t totalPageCount;
uint64_t *freePageStack, *freePageStackBase;

#define PAGE_SIZE KB(4)
#define PAGE_STACK_SIZE (totalPageCount * sizeof(uint64_t))

bool memSearch_isFree(uint64_t addr);

#endif /* end of include guard: _PRIV_PHYS_MEM_MANAGER_H_ */
