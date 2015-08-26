#ifndef _K_MALLOC_H_
#define _K_MALLOC_H_

#include "types.h"

void kmalloc_init();
void* kmalloc(size_t size);
void kfree(void* addr);
void kcompact();

#endif /* end of include guard: _K_MALLOC_H_ */
