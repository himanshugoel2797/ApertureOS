#include "syscall_man.h"
#include "syscalls.h"

void syscall_GetSysInfo(void *args)
{
	generic_syscall_3 *syscall = (generic_syscall_3*)args;	
	switch(syscall->arg0)
	{
		case APEROS_PROC_DATA:
		switch(syscall->arg1)
		{
			case APEROS_PROC_ID:
				syscall->retval = ProcessManager_GetCurPID();
			break;
			default:
			syscall->retval = -1;
		}
		break;
		default:
			syscall->retval = -1;
	}
	return;
}