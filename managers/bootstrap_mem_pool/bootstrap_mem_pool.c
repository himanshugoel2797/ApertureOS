#include "bootstrap_mem_pool.h"

uint8_t mem_pool[BOOTSTRAP_MEM_POOL];
uint32_t pos = 0;

void* Bootstrap_malloc(size_t size)
{
    if( (pos + size) > BOOTSTRAP_MEM_POOL )
}
