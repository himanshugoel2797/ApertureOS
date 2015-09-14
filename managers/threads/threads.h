#ifndef _THREAD_MANAGER_H_
#define _THREAD_MANAGER_H_

#include "types.h"
#include "managers.h"
#include "idt.h"

typedef struct Thread
{
    uint64_t* cr3;
    Registers regs;
    char FPU_state[512 + 64]; //Allocate extra space for alignment
    UID uid;
    uint32_t status;
    uint32_t flags;
    uint32_t kstack;
    struct Thread *next;
} Thread;

typedef enum
{
    THREAD_FLAGS_NONE = 0,
    THREAD_FLAGS_USER = 0,
    THREAD_FLAGS_KERNEL = 1,
    THREAD_FLAGS_FORK = 2,
    THREAD_FLAGS_VM86 = 4
};

void ThreadMan_Setup();
UID ThreadMan_CreateThread(ProcessEntryPoint entry, int argc, char** argv, uint32_t flags);
void ThreadMan_StartThread(UID id);
void ThreadMan_ExitThread(UID id);
void ThreadMan_DeleteThread(UID id);
UID ThreadMan_GetCurThreadID();
void ThreadMan_Yield();

#endif /* end of include guard: _THREAD_MANAGER_H_ */
