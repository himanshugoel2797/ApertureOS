#include "process_man.h"
#include "kmalloc.h"
#include "utils/common.h"

ProcessInfo *processes;

void
ProcessManager_Initialize(void)
{
    //Create the OS process
    processes = kmalloc(sizeof(ProcessInfo));
    processes->flags |= PROC_PERM_KERNEL;
    processes->uid = new_uid();
    strcpy(processes->name, "kernel");
    processes->path = NULL;
    processes->children = NULL;
    processes->next = NULL;

    uint32_t* ktls = (uint32_t*)ThreadMan_GetCurThreadTLS();
    ktls[0] = (uint32_t)processes;	//Store the process pointer in the first uint in the ktls


}

void
ProcessManager_BootstrapProcess(int argc, char **argv)
{
    //Get the current process ID
    ProcessInfo* p_inf = ProcessManager_GetCurProcessInfo ();

    //We would want to copy the args into the KTLS so they can be pulled out later

    UID tmp = Elf_Load(p_inf->path, (p_inf->flags & 2)?ELF_KERNEL:ELF_USER);
    if(tmp != -1 && tmp != -2)Elf_Start(tmp);

}

UID
ProcessManager_CreateProcess(const char *name,
                             const char *path,
                             uint32_t argc,
                             char **argv,
                             ProcessInfo *parent,
                             uint32_t flags)
{
    ThreadMan_Lock();   //Can't have thread switches happening here

    ProcessInfo *proc = kmalloc(sizeof(ProcessInfo));

    proc->uid = new_uid();
    proc->flags = flags | PROC_STATUS_RUNNING;
    memset(proc->name,
           0,
           MAX_PROC_NAME_LEN);

    memcpy(proc->name,
           name,
           (strlen(name) > MAX_PROC_NAME_LEN - 1)?MAX_PROC_NAME_LEN - 1: strlen(name));

    proc->path = kmalloc(strlen(path) + 1);
    strcpy(proc->path, path);

    
    //Add to the process tree, all processes are under the kernel
    if(parent == NULL)parent = processes;
    proc->parent = parent;
    if(parent->children == NULL)
        {
            parent->children = proc;
        }
    else
        {
            proc->next = parent->children->next;
            parent->children->next = proc;
        }

    UID tid = ThreadMan_CreateThread(ProcessManager_BootstrapProcess,
                                     argc,
                                     argv,
                                     (flags & 2)?THREAD_FLAGS_KERNEL:THREAD_FLAGS_USER );

    uint32_t* ktls = (uint32_t*)ThreadMan_GetThreadTLS(tid);
    ktls[0] = (uint32_t)proc;


    ThreadMan_StartThread(tid);
    ThreadMan_Unlock();
}

uint32_t
ProcessManager_CreateProcess_Syscall(void *param)
{

}


UID
ProcessManager_ForkProcess(UID to_fork)
{

}

uint32_t
ProcessManager_ForkProcess_Syscall(void *param)
{

}

ProcessInfo*
ProcessManager_GetCurProcessInfo(void)
{
    uint32_t* ktls = (uint32_t*)ThreadMan_GetCurThreadTLS();
    return (ProcessInfo*)ktls[0];
}

UID
ProcessManager_GetCurPID(void)
{
    return ProcessManager_GetCurProcessInfo()->uid;
}