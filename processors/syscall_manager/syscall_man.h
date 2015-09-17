#ifndef _SYSCALL_MAN_PROC_H_
#define _SYSCALL_MAN_PROC_H_

#include "types.h"
#include "drivers.h"
#include "managers.h"
#include "processors.h"

#define MAX_SYSCALLS 256
#define VAR_SYSCALL_ARGS (uint32_t)(-1)

typedef struct{
	uint32_t size;
	uint32_t arg0;
	uint32_t retval;
}generic_syscall;

typedef struct{
	uint32_t size;
	uint32_t arg0;
	uint32_t arg1;
	uint32_t retval;
}generic_syscall_2;

typedef struct{
	uint32_t size;
	uint32_t arg0;
	uint32_t arg1;
	uint32_t arg2;
	uint32_t retval;
}generic_syscall_3;

typedef void(*SyscallHandler)(void *args);

typedef enum{
	ERROR_NO_FREE_SLOTS = 1
}SyscallRegisterError;

void
SyscallManager_Initialize(void);

void
SyscallManager_RegisterSyscall(uint32_t syscall_ID, 
                               SyscallHandler handler,
                               uint32_t argc);


SyscallRegisterError
SyscallManager_RegisterAppendSyscall(SyscallHandler handler,
                                     uint32_t argc);

#endif