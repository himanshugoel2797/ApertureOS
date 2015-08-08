#ifndef _MEM_PAGING_H_
#define _MEM_PAGING_H_

#include <stddef.h>
#include <stdint.h>

void Paging_Initialize();
void Paging_MapPage(uint64_t physAddress, uint32_t virtualAddress, int userAccessible, int pse);
void Paging_Flush(void *addr);
void Paging_FlushAll();

#endif /* end of include guard: _MEM_PAGING_H_ */
