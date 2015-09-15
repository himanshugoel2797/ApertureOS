#ifndef _SYSCALL_MAN_PROC_H_
#define _SYSCALL_MAN_PROC_H_

#include "types.h"
#include "drivers.h"
#include "managers.h"
#include "processors.h"

#define MAX_SYSCALLS 256

typedef void(*SyscallHandler)(void *args);

void
SyscallManager_Initialize(void);

void
SyscallManager_RegisterSyscall(uint32_t syscall_ID, 
                               SyscallHandler handler);

#endif