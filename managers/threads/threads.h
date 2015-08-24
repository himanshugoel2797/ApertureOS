#ifndef _THREAD_MANAGER_H_
#define _THREAD_MANAGER_H_

#include "types.h"
#include "managers.h"
#include "idt.h"

typedef struct Thread {
        Registers regs;
        uint32_t cr3;
        UID uid;
        bool isKernelMode;
        struct Thread *next;
}Thread;

void ThreadMan_Setup();
UID ThreadMan_CreateThread(ProcessEntryPoint entry, int argc, char** argv, bool isKernelMode);
void ThreadMan_StartThread(UID id);
void ThreadMan_ExitThread(UID id);
void ThreadMan_DeleteThread(UID id);
UID ThreadMan_GetCurThreadID();

#endif /* end of include guard: _THREAD_MANAGER_H_ */
