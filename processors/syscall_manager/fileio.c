#include "syscalls.h"
#include "syscall_man.h"

void
syscall_open(void *args)
{
    generic_syscall_2 *syscall = (generic_syscall_2*)args;
    syscall->retval = -1;

    syscall->retval = Filesystem_OpenFile(syscall->arg0, 0, 0) + 3;
    return;
}

void
syscall_write(void *args)
{
    generic_syscall_3 *a = (generic_syscall_3*)args;

    if(a->arg0 == 0 || a->arg0 == 1 || a->arg0 == 2)
    {
    	char *l = a->arg1;
    	Terminal_Write(l, a->arg2);
	}
	return;
}

void
syscall_close(void *args)
{
	generic_syscall *syscall = (generic_syscall*)args;
	if(syscall->arg0 >= 3)Filesystem_CloseFile(syscall->arg0 - 3);
}