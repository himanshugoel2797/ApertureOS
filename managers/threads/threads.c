#include "threads.h"
#include "drivers.h"
#include "managers.h"
#include "utils/common.h"

SystemData *thread_sys = NULL;
uint32_t threadMan_Initialize();
uint8_t threadMan_messageHandler(Message *msg);

Thread *threads, *curThread, *lastThread;

void ThreadMan_Setup()
{
        thread_sys = SysMan_RegisterSystem();
        strcpy(thread_sys->sys_name, "threadMan");

        thread_sys->prerequisites[0] = Interrupts_GetSysID();
        thread_sys->prerequisites[1] = 0;     //No prereqs

        thread_sys->init = threadMan_Initialize;
        thread_sys->init_cb = NULL;
        thread_sys->msg_cb = threadMan_messageHandler;

        SysMan_StartSystem(thread_sys->sys_id);
}

uint32_t threadMan_InterruptHandler(Registers *regs)
{
        Thread *nxThread = curThread->next;
        if(nxThread == NULL) nxThread = threads;

        curThread->regs.ds = regs->ds;
        curThread->regs.edi = regs->edi;
        curThread->regs.esi = regs->esi;
        curThread->regs.ebp = regs->ebp;
        curThread->regs.unused = regs->unused;
        curThread->regs.ebx = regs->ebx;
        curThread->regs.edx = regs->edx;
        curThread->regs.ecx = regs->ecx;
        curThread->regs.eax = regs->eax;
        curThread->regs.int_no = regs->int_no;
        curThread->regs.err_code = regs->err_code;
        curThread->regs.eip = regs->eip;
        curThread->regs.cs = regs->cs;
        curThread->regs.eflags = regs->eflags;
        curThread->regs.useresp = regs->useresp;
        curThread->regs.ss = regs->ss;
        asm volatile ("mov %%cr3, %0" : "=r" (curThread->cr3));

        regs->ds = nxThread->regs.ds;
        regs->edi = nxThread->regs.edi;
        regs->esi = nxThread->regs.esi;
        regs->ebp = nxThread->regs.ebp;
        regs->unused = nxThread->regs.unused;
        regs->ebx = nxThread->regs.ebx;
        regs->edx = nxThread->regs.edx;
        regs->ecx = nxThread->regs.ecx;
        regs->eax = nxThread->regs.eax;
        regs->int_no = nxThread->regs.int_no;
        regs->err_code = nxThread->regs.err_code;
        regs->eip = nxThread->regs.eip;
        regs->cs = nxThread->regs.cs;
        regs->eflags = nxThread->regs.eflags;
        regs->useresp = nxThread->regs.useresp;
        regs->ss = nxThread->regs.ss;
        asm volatile ("mov %0, %%cr3" :: "r" (nxThread->cr3));

        curThread = nxThread;

        return 1; //Stop any further handlers from executing
}

uint32_t threadMan_Initialize()
{
        Interrupts_RegisterHandler(48, 0, threadMan_InterruptHandler);
}

uint8_t threadMan_messageHandler(Message *msg)
{

}

void ThreadMan_CreateThread(Thread *thread, ProcessEntryPoint entry)
{

}

UID ThreadMan_StartThread(Thread *thread)
{

}

void ThreadMan_ExitThread(UID id)
{

}

void ThreadMan_DeleteThread(UID id)
{

}
