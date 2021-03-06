#ifndef _THREAD_MANAGER_H_
#define _THREAD_MANAGER_H_

#include "types.h"
#include "managers.h"
#include "idt/idt.h"

#define MAX_K_SEMAPHORE_COUNT 2048

typedef struct
{
    void *proc_info;
    void *sock_info;
} K_TLS;

typedef struct Thread
{
    struct Thread *next;
    Registers regs;
    uint64_t* cr3;
    uint32_t kstack;
    uint32_t status;
    UID uid;
    uint32_t flags;
    K_TLS k_tls;
    char *FPU_state; //Allocate extra space for alignment
} __attribute__((packed)) Thread;

typedef enum
{
    THREAD_FLAGS_NONE = 0,
    THREAD_FLAGS_USER = 0,
    THREAD_FLAGS_KERNEL = 1,
    THREAD_FLAGS_FORK = 2,
    THREAD_FLAGS_VM86 = 4
} THREAD_FLAGS;

void
ThreadMan_Setup(void);

UID
ThreadMan_CreateThread(ProcessEntryPoint entry,
                       int argc,
                       char** argv,
                       uint32_t flags);

void
ThreadMan_StartThread(UID id);

void
ThreadMan_SuspendThread(UID id);

void
ThreadMan_ResumeThread(UID id);

void
ThreadMan_ExitThread(UID id);

void
ThreadMan_DeleteThread(UID id);

UID
ThreadMan_GetCurThreadID(void);

K_TLS*
ThreadMan_GetCurThreadTLS(void);

void
ThreadMan_Yield(void);

void
ThreadMan_Lock(void);

void
ThreadMan_Unlock(void);

K_TLS*
ThreadMan_GetThreadTLS(UID id);


UID
ThreadMan_CreateSemaphore(uint32_t count);

bool
ThreadMan_TryAcquireSemaphore(UID id);

bool
ThreadMan_WaitAcqureSemaphore(UID id);

void
ThreadMan_ReleaseSemaphore(UID id);

uint32_t
ThreadMan_DeleteSemaphore(UID id);

#endif /* end of include guard: _THREAD_MANAGER_H_ */
