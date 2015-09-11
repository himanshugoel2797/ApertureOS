#ifndef _U_MALLOC_H_
#define _U_MALLOC_H_

#include "types.h"

void umalloc_init();
void* umalloc(size_t size);
void ufree(void* addr);
void ucompact();

#endif /* end of include guard: _K_MALLOC_H_ */
