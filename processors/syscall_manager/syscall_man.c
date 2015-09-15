#include "syscall_man.h"
#include "utils/common.h"
#include "idt.h"

SyscallHandler syscalls[MAX_SYSCALLS];

uint32_t
SyscallManager_SyscallRaised(Registers *regs)
{
	#ifdef LOG_SYSCALL
	COM_WriteStr("Syscall %d raised!\r\n", regs->ebx);
	#endif
	if(regs->ebx < MAX_SYSCALLS && syscalls[regs->ebx] != NULL)
		syscalls[regs->ebx]((void*)regs->ecx);
}

void
SyscallManager_Initialize(void)
{
	memset(syscalls, 0, sizeof(SyscallHandler) * MAX_SYSCALLS);
    IDT_SetEntry(0x80, (uint32_t)idt_handlers[0x80], 0x08, 0xEE);
	Interrupts_RegisterHandler(0x80, 0, SyscallManager_SyscallRaised);
}

void
Syscall_RegisterSyscall(uint32_t syscall_ID,
                        SyscallHandler handler)
{
	if(syscalls[syscall_ID] != NULL)COM_WriteStr("WARNING Syscall overwrite!\r\n");
	syscalls[syscall_ID] = handler;
}

