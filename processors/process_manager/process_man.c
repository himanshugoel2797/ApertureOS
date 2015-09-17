#include "process_man.h"
#include "kmalloc.h"
#include "utils/common.h"

ProcessInfo *processes;

void
ProcessManager_Initialize(void)
{
	//Create the OS process
	process = kmalloc(sizeof(ProcessInfo));
	process->flags |= PROC_PERM_KERNEL;
	process->uid = new_uid();
	strcpy(process->name, "kernel");
	process->path = NULL;
	process->children = NULL;
	process->next = NULL;

	uint32_t* ktls = (uint32_t*)ThreadMan_GetCurThreadTLS();
	ktls[0] = (uint32_t)process;	//Store the process pointer in the first uint in the ktls


}

void
ProcessManager_BootstrapProcess(int argc, char **argv)
{

}

UID
ProcessManager_CreateProcess(const char *name,
                             const char *path,
                             uint32_t argc,
                             char **argv,
                             ProcessInfo *parent,
                             uint32_t flags)
{
	ThreadMan_Lock();	//Can't have thread switches happening here
	
	ProcessInfo proc = kmalloc(sizeof(ProcessInfo));

	proc->uid = new_uid();
	proc->flags = flags;
	memset(proc->name, 
	       0, 
	       MAX_PROC_NAME_LEN);

	memcpy(proc->name, 
	       name, 
	       (strlen(name) > MAX_PROC_NAME_LEN - 1)?MAX_PROC_NAME_LEN - 1: strlen(name));



	UID tid = ThreadMan_CreateThread(ProcessManager_BootstrapProcess, 
	                                 argc, 
	                                 argv, 
	                                 (flags & 2)?THREAD_FLAGS_KERNEL:THREAD_FLAGS_USER );

	uint32_t* ktls = (uint32_t*)ThreadMan_GetCurThreadTLS();
	ktls[0] = (uint32_t)proc;

	ThreadMan_Unlock();
}

uint32_t
ProcessManager_CreateProcess_Syscall(void *param)
{
	//TODO setup TLS and allocate a structure in it for the process info pointer
	//the TLS is used to keep track of the current process info, however this introduces a vulnerability, a thread might overwrite the pointer in its TLS with another pointer, 
}


UID
ProcessManager_ForkProcess(UID to_fork)
{

}

uint32_t
ProcessManager_ForkProcess_Syscall(void *param)
{

}


UID
ProcessManager_GetCurPID(void)
{
	uint32_t* ktls = (uint32_t*)ThreadMan_GetCurThreadTLS();
	return ((ProcessInfo*)ktls[0])->uid;
}

uint32_t
ProcessManager_GetCurPID_Syscall(void *param)
{

}