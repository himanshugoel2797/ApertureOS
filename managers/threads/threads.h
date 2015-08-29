#ifndef _THREAD_MANAGER_H_
#define _THREAD_MANAGER_H_

#include "types.h"
#include "managers.h"
#include "idt.h"

typedef struct Thread {
        VirtMemMan_Instance cr3;
        char FPU_state[512];
        Registers regs;
        UID uid;
        uint64_t flags;
        struct Thread *next;
}Thread;

typedef enum{
	THREAD_FLAGS_NONE = 0,
	THREAD_FLAGS_USER = 0,
	THREAD_FLAGS_KERNEL = 1,
	THREAD_FLAGS_FORK = 2,
	THREAD_FLAGS_VM86 = 4
};

void ThreadMan_Setup();
UID ThreadMan_CreateThread(ProcessEntryPoint entry, int argc, char** argv, uint64_t flags);
void ThreadMan_StartThread(UID id);
void ThreadMan_ExitThread(UID id);
void ThreadMan_DeleteThread(UID id);
UID ThreadMan_GetCurThreadID();

#endif /* end of include guard: _THREAD_MANAGER_H_ */
