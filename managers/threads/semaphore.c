#include "threads.h"
#include "priv_semaphore.h"
#include "utils/common.h"

Semaphore_Inst semaphores[MAX_K_SEMAPHORE_COUNT];
UID semaphore_id_base;

void
ThreadMan_SemaphoreInitialize(void)
{
	memset(semaphores, 0, sizeof(Semaphore_Inst) * MAX_K_SEMAPHORE_COUNT);
	for(uint32_t i = 0; i < MAX_K_SEMAPHORE_COUNT; i++)
	{
		semaphores[i].id = new_uid();
	}
	semaphore_id_base = semaphores[0].id;

}

UID
ThreadMan_CreateSemaphore(uint32_t count)
{
	uint32_t i;
	for(i = 0; i < MAX_K_SEMAPHORE_COUNT; i++)
	{
		if(semaphores[i].max_locks == 0)break;
	}
	if(i == MAX_K_SEMAPHORE_COUNT)return -1;

	semaphores[i].max_locks = count;
	semaphores[i].lock_count = count;
	return semaphores[i].id;
}

bool
ThreadMan_TryAcquireSemaphore(UID id)
{
	uint32_t i = id - semaphore_id_base;
	if(semaphores[i].max_locks == 0)return -2;
	if(semaphores[i].lock_count == 0)return FALSE;

	semaphores[i].lock_count--;
	return TRUE;
}

bool
ThreadMan_WaitAcqureSemaphore(UID id)
{
	while(1){
		uint8_t status = ThreadMan_TryAcquireSemaphore(id);
		if(status == -2)return FALSE;
		if(status == TRUE)return TRUE;
		if(!Interrupts_IsInHandler())ThreadMan_Yield();
		else return FALSE;
	}
}

void
ThreadMan_ReleaseSemaphore(UID id)
{
	uint32_t i = id - semaphore_id_base;
	if(semaphores[i].max_locks == 0)return;
	if(semaphores[i].lock_count == semaphores[i].max_locks)return;

	semaphores[i].lock_count++;
	return;
}

uint32_t
ThreadMan_DeleteSemaphore(UID id)
{
	uint32_t i = id - semaphore_id_base;
	semaphores[i].max_locks = 0;
	semaphores[i].lock_count = 0;
	return 1;
}