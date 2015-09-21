#ifndef _PRIV_SEMAPHORE_H_
#define _PRIV_SEMAPHORE_H_

#include "types.h"
#include "threads.h"

typedef struct{
	UID id;
	uint32_t lock_count;
	uint32_t max_locks;
} Semaphore_Inst;

void
ThreadMan_SemaphoreInitialize(void);

#endif