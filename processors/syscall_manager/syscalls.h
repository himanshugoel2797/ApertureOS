#ifndef AOS_PRIV_SYSCALLS_H_
#define AOS_PRIV_SYSCALLS_H_

#include "types.h"

#define SYSCALL_GETSYSINFO_ARGC 3
void syscall_GetSysInfo(void *args);

#endif