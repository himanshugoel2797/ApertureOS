#ifndef _MEMORY_MANAGER_H_
#define _MEMORY_MANAGER_H_

#include <stddef.h>
#include <stdint.h>

void MemMan_Initialize();
void* MemMan_Alloc(uint64_t size);
void MemMan_Free(void *ptr, uint64_t size);

#endif /* end of include guard: _MEMORY_MANAGER_H_ */
