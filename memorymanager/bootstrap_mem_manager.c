#include "bootstrap_mem_manager.h"

char memoryPool[BOOTSTRAP_POOL_SIZE];
size_t curLoc = 0;

void* Bootstrap_malloc(size_t size)
{
        void *curPos = &memoryPool[curLoc];
        curLoc += (size & ~3);
        if(curLoc >= BOOTSTRAP_POOL_SIZE) return NULL;
        return curPos;
}
