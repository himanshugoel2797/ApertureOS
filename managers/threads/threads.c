#include "threads.h"
#include "drivers.h"
#include "managers.h"
#include "utils/common.h"

SystemData *thread_sys = NULL;
uint32_t threadMan_Initialize();
uint8_t threadMan_messageHandler(Message *msg);

Thread *threads, *curThread, *lastThread;
UID uidBase = 0;

void kernel_main(int, char**);

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

__attribute__((naked, noreturn))
void threadMan_IDTHandler()
{
        asm volatile(
                "push $0x00\n\t"
                "push $0x00\n\t"
                "pusha\n\t"
                "mov %ds, %ax\n\t"
                "push %eax\n\t"
                "mov $0x10, %ax\n\t"
                "mov %ax, %ds\n\t"
                "mov %ax, %es\n\t"
                "mov %ax, %fs\n\t"
                "mov %ax, %gs\n\t"
                "push %esp\n\t"
                );

        asm volatile(
                "call threadMan_InterruptHandler"
                );
        asm volatile(
                "pop %ebx\n\t"
                "pop %ebx\n\t"
                "mov %bx, %ds\n\t"
                "mov %bx, %es\n\t"
                "mov %bx, %fs\n\t"
                "mov %bx, %gs\n\t"
                //"hlt\n\t"
                "popa\n\t"
                "add $8, %esp\n\t"
                "push $48\n\t"
                "call APIC_SendEOI\n\t"
                "pop %eax\n\t"
                "iret\n\t"
                );
}

void threadMan_InterruptHandler(Registers *regs)
{       
        COM_WriteStr("RECIEVED THREAD PREEMPT!\r\n");
        if(curThread != NULL) {


                Thread *nxThread = curThread->next;
                if(nxThread == NULL) nxThread = threads;

                /*
                curThread->regs.ds = regs->ds;
                curThread->regs.edi = regs->edi;
                curThread->regs.esi = regs->esi;
                curThread->regs.ebp = regs->ebp;
                curThread->regs.unused = regs;
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
                */
                curThread->regs.unused = regs;
                curThread->regs.unused -= 4;

                /*COM_WriteStr("Register Dump\r\n");
                COM_WriteStr("EAX: %x\t", regs->eax);
                COM_WriteStr("EBX: %x\t", regs->ebx);
                COM_WriteStr("ECX: %x\t", regs->ecx);
                COM_WriteStr("EDX: %x\r\n", regs->edx);
                COM_WriteStr("EIP: %x\t", regs->eip);
                COM_WriteStr("ESI: %x\t", regs->esi);
                COM_WriteStr("EDI: %x\t\r\n", regs->edi);
                COM_WriteStr("CS: %x\t", regs->cs);
                COM_WriteStr("SS: %x\t", regs->ss);
                COM_WriteStr("DS: %x\t\r\n", regs->ds);
                COM_WriteStr("EFLAGS: %b\t\r\n", regs->eflags);
                COM_WriteStr("USERESP: %x\t", regs->useresp);
                COM_WriteStr("EBP: %x\t\r\n", regs->ebp);*/

                /*regs->ds = nxThread->regs.ds;
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
                */


                //memcpy(regs->unused, nxThread->regs, sizeof(Registers));
                //regs->unused -= sizeof(Registers);

                //asm volatile ("mov %0, %%cr3" :: "r" (nxThread->cr3));

                curThread = nxThread;

                //Switch stacks
                asm volatile
                (
                        "mov %%ebx, %%eax\n\t"
                        "add $0x4, %%eax\n\t"   //Calculate the ebp for this function
                        "add $0xc, %%esp\n\t"
                        "pop %%eax\n\t"
                        "pop %%eax\n\t"
                        "mov %%ebx, %%esp\n\t"
                        "push 4(%%ebp)\n\t"    //Push the return address for this function
                        "ret\n\t"
                        :: "b"(nxThread->regs.unused) : "%eax"
                );
        }
        //return 1; //Stop any further handlers from executing
}

uint32_t threadMan_Initialize()
{
        //TODO create a new thread and resume remaining work on that by calling another function, this lets us keep things clean
        curThread = NULL;
        threads = NULL;

        //Force override the IDT entry for this!
        IDT_SetEntry(48, (uint32_t)threadMan_IDTHandler, 0x08, 0x8E);

        //Enable the APIc timer
        APIC_SetTimerMode(APIC_TIMER_PERIODIC);

        APIC_SetTimerValue(1 << 10);


        threads = kmalloc(sizeof(Thread));
        threads->uid = uidBase++;
        threads->flags = THREAD_FLAGS_KERNEL;
        threads->next = NULL;
        lastThread = curThread = threads;

        UID tmp = ThreadMan_CreateThread(kernel_main, 0, NULL, THREAD_FLAGS_KERNEL);

        APIC_SetVector(APIC_TIMER, 48);
        APIC_SetEnableInterrupt(APIC_TIMER, 1);

        //Create the new thread to continue exectuon

        return 0;
}

uint8_t threadMan_messageHandler(Message *msg)
{

}

UID ThreadMan_CreateThread(ProcessEntryPoint entry, int argc, char**argv, uint64_t flags)
{
        //Entering critical section, disable all interrupts
        Interrupts_Lock();
        Thread *curThreadInfo = kmalloc(sizeof(Thread));
        COM_WriteStr("curThreadInfo %d\r\n", curThreadInfo);
        curThreadInfo->uid = uidBase++;
        curThreadInfo->flags = flags;

        //Setup the paging structures for the thread
        curThreadInfo->cr3 = virtMemMan_CreateInstance();
        if((flags & THREAD_FLAGS_FORK) == THREAD_FLAGS_FORK)
        {
                virtMemMan_ForkCurrent(curThreadInfo->cr3);
        }
        //TODO setup the remaining registers to suit, set the args for the function too
        memset(&curThreadInfo->regs, 0, sizeof(Registers));
        memset(curThreadInfo->FPU_state, 0, 512);

        curThreadInfo->regs.eip = entry;
        curThreadInfo->regs.unused = kmalloc(KB(16)) + KB(16); //Stack ptr
        curThreadInfo->regs.ebp = curThreadInfo->regs.unused;

        //Somehow deal with globals here

        //Push args onto the stack by temporarily switching stacks and pushing the stuff
        asm volatile(
                "mov %%esp, %%edi\n\t"
                "mov %%ebx, %%esp\n\t"
                
                //Push parameters
                "push %%eax\n\t"
                "push %%ecx\n\t"
                
                //Push iret stuff
                "sti \n\t"
                "pushf\n\t"
                "cli \n\t"
                "push $0x08\n\t"
                "push %%edx\n\t"
                
                //Push filler
                "push $0x00\n\t"
                "push $0x00\n\t"

                //Push popa
                "push $0x00\n\t"
                "push $0x00\n\t"
                "push $0x00\n\t"
                "push $0x00\n\t"
                "push $0x00\n\t"
                "push %%ebx\n\t"
                "push $0x00\n\t"
                "push $0x00\n\t"

                //Push filler
                "push $0x10\n\t"
                "push $0x00\n\t"

                //Backup and reset
                "mov %%esp, %%ebx\n\t"
                "mov %%edi, %%esp\n\t"
                :: "a"(argv), "b"(curThreadInfo->regs.unused), "c"(argc), "d"(curThreadInfo->regs.eip)
                );
        asm volatile("mov %%ebx, %0" : "=r"(curThreadInfo->regs.unused));
        COM_WriteStr("\r\nSTACK: %x", curThreadInfo->regs.unused);
        //curThreadInfo->regs.unused -= sizeof(uint32_t*);
        //*((uint32_t*)curThreadInfo->regs.unused) = (uint32_t)argv;
        //curThreadInfo->regs.unused -= sizeof(uint32_t*);
        //*((uint32_t*)curThreadInfo->regs.unused) = (uint32_t)argc;

//0x8 + 0x8 + 0xc

        //Store the thread in the queue
        lastThread->next = curThreadInfo;
        lastThread = lastThread->next;

        Interrupts_Unlock();

        return curThreadInfo->uid;
}

void ThreadMan_StartThread(UID id)
{

}

void ThreadMan_ExitThread(UID id)
{

}

void ThreadMan_DeleteThread(UID id)
{

}
