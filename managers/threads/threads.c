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
                "pop %eax\n\t"
                "pop %eax\n\t"
                "mov %ax, %ds\n\t"
                "mov %ax, %es\n\t"
                "mov %ax, %fs\n\t"
                "mov %ax, %gs\n\t"
                //"hlt\n\t"
                "push $48\n\t"
                "call APIC_SendEOI\n\t"
                "pop %eax\n\t"
                "popa\n\t"
                "add $8, %esp\n\t"
                "iret\n\t"
                );
}

void threadMan_InterruptHandler(Registers *regs)
{       
        if(curThread != NULL) {


                Thread *nxThread = curThread->next;
                if(nxThread == NULL) nxThread = threads;

                curThread->regs.unused = regs;
                curThread->regs.unused -= 4;

                curThread = nxThread;

                //Switch stacks
                asm volatile
                (
                        "add $0xc, %%esp\n\t"
                        "pop %%eax\n\t"
                        "add $4, %%esp\n\t"
                        "mov %%ebx, %%esp\n\t"
                        "mov %%eax, %%ebx\n\t"
                        "push 4(%%ebp)\n\t"    //Push the return address for this function
                        "ret\n\t"
                        :: "b"(nxThread->regs.unused) : "%eax"
                );
        }
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

        APIC_SetTimerValue(1 << 20);


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
                
                //Push filler (interrupt info)
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
