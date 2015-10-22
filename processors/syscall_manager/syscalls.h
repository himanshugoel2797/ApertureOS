#ifndef AOS_PRIV_SYSCALLS_H_
#define AOS_PRIV_SYSCALLS_H_

#include "types.h"

#define SYSCALL_GETSYSINFO_ARGC 3
void
syscall_GetSysInfo(void *args);

#define SYSCALL_OPEN_ARGC 2
void
syscall_open(void *args);

#define SYSCALL_WRITE_ARGC 3
void
syscall_write(void *args);

#define SYSCALL_READ_ARGC 3
void
syscall_read(void *args);

#define SYSCALL_CLOSE_ARGC 1
void
syscall_close(void *args);
#endif