#ifndef _BOOTSTRAP_MEM_MANAGER_H_
#define _BOOTSTRAP_MEM_MANAGER_H_

#include <stdint.h>
#include <stddef.h>

#define BOOTSTRAP_POOL_SIZE  1024 * 1024 * 32    //Allocate 32MB of space for early structures

//A very simple watermarking bootstrap memory manager to store important boot time info
void* Bootstrap_malloc(size_t size);

#endif /* end of include guard: _BOOTSTRAP_MEM_MANAGER_H_ */
