#ifndef _THREAD_MANAGER_H_
#define _THREAD_MANAGER_H_

#include "types.h"
#include "managers.h"
#include "idt.h"

typedef struct Thread {
        Registers regs;
        uint32_t cr3;
        struct Thread *next;
}Thread;

void ThreadMan_Setup();
void ThreadMan_CreateThread(Thread *thread, ProcessEntryPoint entry);
UID ThreadMan_StartThread(Thread *thread);
void ThreadMan_ExitThread(UID id);
void ThreadMan_DeleteThread(UID id);

#endif /* end of include guard: _THREAD_MANAGER_H_ */
