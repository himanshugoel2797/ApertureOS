#ifndef _SYSCALL_MAN_PROC_H_
#define _SYSCALL_MAN_PROC_H_

#include "types.h"
#include "drivers.h"
#include "managers.h"
#include "processors.h"

#define MAX_SYSCALLS 256
#define VAR_SYSCALL_ARGS (uint32_t)(-1)

#define GETDATA_SYSCALL_NUM 7       //*< get data about the environment

//* The type of the data to access
typedef enum
{
    APEROS_ENV_DATA = 0,    //*< Get environment data
    APEROS_PROC_DATA = 1    //*< Get process data
} APEROS_DATA_TYPE;

//* The kind of environment data to access
typedef enum
{
    APEROS_ENV_VAR = 1      //*< Get a list of environment variables
} APEROS_ENV;

//* The kind of process data to access
typedef enum
{
    APEROS_PROC_DATA_ARGC = 1,  //*< Get the number of arguments passed to the process
    APEROS_PROC_DATA_ARGV = 2,  //*< Get the values of the args passed to the process
    APEROS_PROC_ID = 3          //*< Get the process ID
} APEROS_PROC;

typedef struct
{
    uint32_t size;
    uint64_t arg0;
    uint64_t retval;
} generic_syscall;

typedef struct
{
    uint32_t size;
    uint64_t arg0;
    uint64_t arg1;
    uint64_t retval;
} generic_syscall_2;

typedef struct
{
    uint32_t size;
    uint64_t arg0;
    uint64_t arg1;
    uint64_t arg2;
    uint64_t retval;
} generic_syscall_3;

typedef void(*SyscallHandler)(void *args);

typedef enum
{
    ERROR_NO_FREE_SLOTS = 1
} SyscallRegisterError;

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