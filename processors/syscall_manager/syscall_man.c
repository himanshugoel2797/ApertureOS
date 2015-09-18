#include "syscall_man.h"
#include "utils/common.h"
#include "idt.h"

typedef struct
{
    SyscallHandler handler;
    uint32_t arg_count;
} SyscallEntry;

uint32_t curIndex;
SyscallEntry syscalls[MAX_SYSCALLS];

uint32_t
SyscallManager_SyscallRaised(Registers *regs)
{
#ifdef LOG_SYSCALL
    COM_WriteStr("Syscall %d raised!\r\n", regs->ebx);
#endif

    if(regs->ebx < MAX_SYSCALLS && syscalls[regs->ebx].handler != NULL)
        {
            //Make sure the proper number of arguments has been provided
            uint32_t *size = (uint32_t*)regs->ecx;
            if(*size !=
                    (syscalls[regs->ebx].arg_count + 1) * sizeof(uint64_t)
                    + sizeof(uint32_t))
                return 0;	//Just return since we have no idea where the retval should be

            syscalls[regs->ebx].handler((void*)regs->ecx);
        }
        
#ifdef LOG_SYSCALL
    else
        {
            COM_WriteStr("Invalid Syscall!!\r\n");
        }
#endif
}

void
SyscallManager_Initialize(void)
{
    memset(syscalls, 0, sizeof(SyscallEntry) * MAX_SYSCALLS);
    IDT_SetEntry(0x80, (uint32_t)idt_handlers[0x80], 0x08, 0xEE);
    Interrupts_RegisterHandler(0x80, 0, SyscallManager_SyscallRaised);
    curIndex = 0;

    //Register all syscalls

}

void
SyscallManager_RegisterSyscall(uint32_t syscall_ID,
                               SyscallHandler handler,
                               uint32_t argc)
{
    if(syscalls[syscall_ID].handler != NULL)COM_WriteStr("WARNING Syscall overwrite!\r\n");
    syscalls[syscall_ID].handler = handler;
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