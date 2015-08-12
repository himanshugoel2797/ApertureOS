#ifndef _BOOTSTRAP_MEM_POOL_MANAGER_H_
#define _BOOTSTRAP_MEM_POOL_MANAGER_H_

#include "types.h"
#include "utils/common.h"

#define BOOTSTRAP_MEM_POOL MB(32)

void* bootstrap_malloc(size_t size);

#endif /* end of include guard: _BOOTSTRAP_MEM_POOL_MANAGER_H_ */
