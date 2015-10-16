#include "syscall_man.h"
#include "syscalls.h"
#include "utils/common.h"
#include "globals.h"
#include "idt.h"

typedef struct
{
    SyscallHandler handler;
    uint32_t arg_count;
} SyscallEntry;

uint32_t curIndex;
SyscallEntry syscalls[MAX_SYSCALLS];

void
syscall_printf(void *args)
{
    generic_syscall_3 *a = (generic_syscall_3*)args;

    char *l = a->arg1;

    for(int i = 0; i < a->arg2; i++)
    {
        COM_WriteStr("%c", *l++);
    }
}

void
syscall_exit(void *args)
{
    ThreadMan_ExitThread(ThreadMan_GetCurThreadID());
    asm volatile("sti\n\t" "int $48");
}

uint32_t
SyscallManager_SyscallRaised(Registers *regs)
{
#ifdef LOG_SYSCALL
    COM_WriteStr("Syscall %d raised!\r\n", regs->ebx);
#endif


    if(regs->ebx < MAX_SYSCALLS && syscalls[regs->ebx].handler != NULL)
        {
            RET_CHECK_PRIV_ADDR(regs->ecx) 0;   //Check and on failure return 0

            //Make sure the proper number of arguments has been provided
            uint32_t size = ((generic_syscall*)regs->ecx)->size;
            if((size == ((syscalls[regs->ebx].arg_count + 1) * sizeof(uint64_t) + sizeof(uint32_t))) |
                    syscalls[regs->ebx].arg_count == VAR_SYSCALL_ARGS)
                syscalls[regs->ebx].handler((void*)regs->ecx);
            else
                return 0;   //Just return since we have no idea where the retval should be

        }

#ifdef LOG_SYSCALL
    else
        {
            COM_WriteStr("Invalid Syscall!!\r\n");
        }
#endif
    return 0;
}

void
SyscallManager_Initialize(void)
{
    memset(syscalls, 0, sizeof(SyscallEntry) * MAX_SYSCALLS);
    IDT_SetEntry(0x80, (uint32_t)idt_handlers[0x80], 0x08, 0xEE);
    Interrupts_RegisterHandler(0x80, 0, SyscallManager_SyscallRaised);
    curIndex = 0;

    //Register all syscalls
    SyscallManager_RegisterSyscall(GETDATA_SYSCALL_NUM, syscall_GetSysInfo, SYSCALL_GETSYSINFO_ARGC);
    SyscallManager_RegisterSyscall(6, syscall_printf, 3);
    SyscallManager_RegisterSyscall(10, syscall_exit, 1);
}

void
SyscallManager_RegisterSyscall(uint32_t syscall_ID,
                               SyscallHandler handler,
                               uint32_t argc)
{
    if(syscalls[syscall_ID].handler != NULL)COM_WriteStr("WARNING Syscall overwrite!\r\n");
    syscalls[syscall_ID].handler = handler;
    syscalls[syscall_ID].arg_count = argc;
}

SyscallRegisterError
SyscallManager_RegisterAppendSyscall(SyscallHandler handler,
                                     uint32_t argc)
{

    while(syscalls[curIndex].handler != NULL && curIndex < MAX_SYSCALLS)
        curIndex++;


    if(syscalls[curIndex].handler != NULL)return ERROR_NO_FREE_SLOTS;

    syscalls[curIndex].handler = handler;
    syscalls[curIndex].arg_count = argc;
    curIndex++;
}

