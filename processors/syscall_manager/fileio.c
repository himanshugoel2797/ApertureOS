#include "syscalls.h"
#include "syscall_man.h"

void
syscall_open(void *args)
{
    generic_syscall_2 *syscall = (generic_syscall_2*)args;
    syscall->retval = -1;

    syscall->retval = (Filesystem_OpenFile(syscall->arg0, 0, 0) + 3);
//            COM_WriteStr("OPEN FILE Addr: %x, FD: %x\r\n", (uint32_t)syscall->arg1, (uint32_t)syscall->retval);
    return;
}

void
syscall_write(void *args)
{
    generic_syscall_3 *a = (generic_syscall_3*)args;

    if(a->arg0 == 1 || a->arg0 == 2)
        {
            char *l = a->arg1;
            Terminal_Write(l, a->arg2);
        }
    return;
}

void
syscall_read(void *args)
{
    generic_syscall_3 *a = (generic_syscall_3*)args;

    a->retval = a->arg2 - 1;

    if(a->arg0 != 0)
        {
//            COM_WriteStr("READ FILE Addr: %x, FD: %x\r\n", (uint32_t)a->arg1, (uint32_t)a->arg0);
            a->retval = Filesystem_ReadFile((uint32_t)(a->arg0 - 3), (uint32_t)a->arg1, (uint32_t)a->arg2);
//            COM_WriteStr("Done! read: %d, requested: %d", (uint32_t)a->retval, (uint32_t)a->arg2);
        }
    return;
}

void
syscall_close(void *args)
{
    generic_syscall *syscall = (generic_syscall*)args;
    if(syscall->arg0 >= 3)Filesystem_CloseFile(syscall->arg0 - 3);
}