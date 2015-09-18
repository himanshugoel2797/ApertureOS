#include "threads.h"
#include "drivers.h"
#include "managers.h"
#include "utils/common.h"
#include "gdt.h"

SystemData *thread_sys = NULL;
uint32_t threadMan_Initialize();
uint8_t threadMan_messageHandler(Message *msg);

uint32_t lock_num = 0;
bool thread_lock = FALSE;

Thread *threads, *curThread, *lastThread;
UID uidBase = 0;

void kernel_main(int, char**);

void
ThreadMan_Setup(void)
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
void
threadMan_IDTHandler()
{
    asm volatile(
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
        "popa\n\t"
        "sub $0x20, %esp\n\t"
        "push $48\n\t"
        "call APIC_SendEOI\n\t"
        "pop %eax\n\t"
        "popa\n\t"
        "iret\n\t"
    );
}

void
threadMan_InterruptHandler(Registers *regs)
{
    if(thread_lock)return;  //Thread switching has been disabled, keep running current thread

    Thread *nxThread = curThread->next;

    while( (nxThread->status & 1) == 0)
        {
            nxThread = nxThread->next;
        }


    uint32_t addr = curThread->FPU_state;
    addr += 64;
    addr -= (addr % 64);
    asm volatile("push %%eax\n\t"
                 "push %%edx\n\t"
                 "mov $0xffffffff, %%eax\n\t"
                 "mov $0xffffffff, %%edx\n\t"
                 "xsave (%%ecx)\n\t"
                 "pop %%edx\n\t"
                 "pop %%eax\n\t" :: "c"(addr));

    curThread->regs.unused = regs;
    curThread->regs.unused -= 4;
    curThread->kstack = sys_tss.esp0;

    //First backup the current page table pointer
    curThread->cr3 = virtMemMan_GetCurrent();

    addr = nxThread->FPU_state;
    addr += 64;
    addr -= (addr % 64);
    asm volatile("push %%eax\n\t"
                 "push %%edx\n\t"
                 "mov $0xffffffff, %%eax\n\t"
                 "mov $0xffffffff, %%edx\n\t"
                 "xrstor (%%ecx)\n\t"
                 "pop %%edx\n\t"
                 "pop %%eax\n\t" :: "c"(addr));
    sys_tss.esp0 = nxThread->kstack;

    //Switch stacks
    curThread = nxThread;

    asm volatile
    (
        "mov %%ebp, %%esp\n\t"
        "pop %%ecx\n\t"
        "pop %%ecx\n\t"
        //"mov 4(%%ebp), %%ecx\n\t"
        "mov %%ebx, %%cr3\n\t"  //Force update this manually
        "mov %%eax, %%esp\n\t"
        "push %%ecx\n\t"
        "push %%ebx\n\t"        //New stack is setup nicely, officially switch now
        "call virtMemMan_SetCurrent\n\t"
        "pop %%ebx\n\t"
        "ret\n\t"
        :: "a"(nxThread->regs.unused), "b"(nxThread->cr3)
    );
}

uint32_t
threadMan_Initialize(void)
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
    threads->status = 0;
    threads->next = threads;
    lastThread = curThread = threads;

    UID tmp = ThreadMan_CreateThread(kernel_main, 0, NULL, THREAD_FLAGS_KERNEL);
    ThreadMan_StartThread(tmp);

    APIC_SetVector(APIC_TIMER, 48);
    APIC_SetEnableInterrupt(APIC_TIMER, 1);

    //Create the new thread to continue exectuon

    return 0;
}

uint8_t
threadMan_messageHandler(Message *msg)
{

}

UID
ThreadMan_CreateThread(ProcessEntryPoint entry,
                       int argc,
                       char**argv,
                       uint32_t flags)
{
    //Entering critical section, disable all interrupts
    Interrupts_Lock();
    Thread *curThreadInfo = kmalloc(sizeof(Thread));
    curThreadInfo->uid = new_uid();
    curThreadInfo->flags = flags;
    curThreadInfo->k_tls = kmalloc(256);
    //curThreadInfo->FPU_state = kmalloc(4096 + 64);
    //COM_WriteStr("%x\r\n", curThreadInfo->k_tls);
    curThreadInfo->status = 0;

    //Setup the paging structures for the thread
    curThreadInfo->cr3 = virtMemMan_CreateInstance();
    if((flags & THREAD_FLAGS_FORK) == THREAD_FLAGS_FORK)
        {
            virtMemMan_ForkCurrent(curThreadInfo->cr3);
        }
    //TODO setup the remaining registers to suit, set the args for the function too
    memset(&curThreadInfo->regs, 0, sizeof(Registers));
    //memset(curThreadInfo->FPU_state, 0, 4096 + 64);

    curThreadInfo->regs.eip = entry;

    //Allocate a kernel mode stack for each thread
    curThreadInfo->kstack = kmalloc(KB(16));

    //uint64_t* prev = virtMemMan_SetCurrent(curThreadInfo->cr3);

    //if((flags & THREAD_FLAGS_KERNEL) == 0){

    //Map the stack into another place for now

    curThreadInfo->regs.unused = 0x50004000; //Stack ptr
    virtMemMan_Map(curThreadInfo->regs.unused - KB(4), physMemMan_Alloc(), KB(4), MEM_TYPE_WB, MEM_READ | MEM_WRITE, (flags & THREAD_FLAGS_KERNEL)?MEM_KERNEL : MEM_USER);
    virtMemMan_Map(curThreadInfo->regs.unused - KB(8), physMemMan_Alloc(), KB(4), MEM_TYPE_WB, MEM_READ | MEM_WRITE, (flags & THREAD_FLAGS_KERNEL)?MEM_KERNEL : MEM_USER);
    virtMemMan_Map(curThreadInfo->regs.unused - KB(12), physMemMan_Alloc(), KB(4), MEM_TYPE_WB, MEM_READ | MEM_WRITE, (flags & THREAD_FLAGS_KERNEL)?MEM_KERNEL : MEM_USER);
    virtMemMan_Map(curThreadInfo->regs.unused - KB(16), physMemMan_Alloc(), KB(4), MEM_TYPE_WB, MEM_READ | MEM_WRITE, (flags & THREAD_FLAGS_KERNEL)?MEM_KERNEL : MEM_USER);
    virtMemMan_Map(curThreadInfo->regs.unused, physMemMan_Alloc(), KB(4), MEM_TYPE_WB, MEM_READ | MEM_WRITE, (flags & THREAD_FLAGS_KERNEL)?MEM_KERNEL : MEM_USER);

    //}else{
    //    curThreadInfo->regs.unused = kmalloc(KB(16)) + KB(16);
    //}

    curThreadInfo->regs.ebp = curThreadInfo->regs.unused;


    //Push args onto the stack by temporarily switching stacks and pushing the stuff
    asm volatile(
        "xchg %%ebx, %%esp\n\t"

        //Push parameters
        "push %%ecx\n\t"
        "push %%eax\n\t"

        //Push iret stuff
        //"push $0xAAAAAAAA\n\t"
        "pushf\n\t"
        "pop %%eax\n\t"
        "or $512, %%eax\n\t"
        "push %%eax\n\t"
        "push $0x08\n\t"
        "push %%edx\n\t"

        //Push popa
        "push $0xAAAAAAAA\n\t"
        "push $0xBBBBBBBB\n\t"
        "push $0xCCCCCCCC\n\t"
        "push $0xDDDDDDDD\n\t"
        "push $0xEEEEEEEE\n\t"
        "push $0xFFFFFFFF\n\t"
        "push $0xA0A0A0A0\n\t"
        "push $0xABABABAB\n\t"

        //Push filler
        "push $0x10\n\t"
        "push $0xABCDABCD\n\t"

        //Backup and reset
        "xchg %%esp, %%ebx\n\t"
        :
        : "a"(argv), "b"(curThreadInfo->regs.unused), "c"(argc), "d"(curThreadInfo->regs.eip)
    );
    asm volatile("mov %%ebx, %0\n\t" : "=r"(curThreadInfo->regs.unused));

    //virtMemMan_SetCurrent(prev);

    //Everything's setup, now setup the address space with the stack without setting it as current
    //if((flags & THREAD_FLAGS_KERNEL) == 0){

    //Map the stack into another place for now
    uint32_t stack_vaddr = 0x40004000;

    curThreadInfo->regs.ebp = stack_vaddr;
    curThreadInfo->regs.unused -= 0x10000000; //Stack ptr

    virtMemMan_MapInst(curThreadInfo->cr3,
                       stack_vaddr - KB(4),
                       virtMemMan_GetPhysAddress((void*)(0x50004000 - KB(4))),
                       KB(4),
                       MEM_TYPE_WB, MEM_READ | MEM_WRITE, (flags & THREAD_FLAGS_KERNEL)?MEM_KERNEL : MEM_USER);


    virtMemMan_MapInst(curThreadInfo->cr3,
                       stack_vaddr - KB(8),
                       virtMemMan_GetPhysAddress((void*)(0x50004000 - KB(8))),
                       KB(4),
                       MEM_TYPE_WB, MEM_READ | MEM_WRITE, (flags & THREAD_FLAGS_KERNEL)?MEM_KERNEL : MEM_USER);


    virtMemMan_MapInst(curThreadInfo->cr3,
                       stack_vaddr - KB(12),
                       virtMemMan_GetPhysAddress((void*)(0x50004000 - KB(12))),
                       KB(4),
                       MEM_TYPE_WB, MEM_READ | MEM_WRITE, (flags & THREAD_FLAGS_KERNEL)?MEM_KERNEL : MEM_USER);


    virtMemMan_MapInst(curThreadInfo->cr3,
                       stack_vaddr - KB(16),
                       virtMemMan_GetPhysAddress((void*)(0x50004000 - KB(16))),
                       KB(4),
                       MEM_TYPE_WB, MEM_READ | MEM_WRITE, (flags & THREAD_FLAGS_KERNEL)?MEM_KERNEL : MEM_USER);


    virtMemMan_MapInst(curThreadInfo->cr3,
                       stack_vaddr,
                       virtMemMan_GetPhysAddress((void*)(0x50004000)),
                       KB(4),
                       MEM_TYPE_WB, MEM_READ | MEM_WRITE, (flags & THREAD_FLAGS_KERNEL)?MEM_KERNEL : MEM_USER);


    virtMemMan_UnMap((void*)(0x50000000), KB(20));


    //virtMemMan_SetCurrent(curThreadInfo->cr3);
    //while(1);
    //}


    uint32_t addr = curThreadInfo->FPU_state;
    addr += 64;
    addr -= (addr % 64);
    asm volatile("push %%eax\n\t"
                 "push %%edx\n\t"
                 "mov $0xffffffff, %%eax\n\t"
                 "mov $0xffffffff, %%edx\n\t"
                 "xsave (%%ecx)\n\t"
                 "pop %%edx\n\t"
                 "pop %%eax\n\t" :: "c"(addr));


    //Store the thread in the queue
    curThreadInfo->next = threads;
    lastThread->next = curThreadInfo;
    lastThread = curThreadInfo;

    Interrupts_Unlock();

    return curThreadInfo->uid;
}

void
ThreadMan_StartThread(UID id)
{
    ThreadMan_Lock();
    Thread *thd = threads;
    do
        {
            if(thd->uid == id)break;
            thd = thd->next;
        }
    while(thd != NULL);

    thd->status |= 1;
    ThreadMan_Unlock();
}

void
ThreadMan_ExitThread(UID id)
{
    ThreadMan_Lock();
    Thread *thd = threads;
    do
        {
            if(thd->uid == id)break;
            thd = thd->next;
        }
    while(thd != NULL);

    thd->status &= ~1;
    ThreadMan_Unlock();
}

void
ThreadMan_DeleteThread(UID id)
{
    if(id == ThreadMan_GetCurThreadID())return; //The current thread object will automatically be cleaned by the kernel

    ThreadMan_Lock();
    Thread *thd = threads, *prev = NULL;
    do
        {
            if(thd->uid == id)break;

            prev = thd;
            thd = thd->next;
        }
    while(thd != NULL);

    if( (thd->status & 1) == 0)
        {
            prev->next = thd->next;
            virtMemMan_FreeInstance(thd->cr3);
            kfree(thd);
        }
    ThreadMan_Unlock();
}

void
ThreadMan_Yield(void)
{
    asm volatile("int $48");
}

UID
ThreadMan_GetCurThreadID(void)
{
    return curThread->uid;
}

void*
ThreadMan_GetCurThreadTLS(void)
{
    return curThread->k_tls;
}

void*
ThreadMan_GetThreadTLS(UID id)
{
    Thread *thd = threads;
    do
        {
            if(thd->uid == id)break;
            thd = thd->next;
        }
    while(thd != NULL);
    return thd->k_tls;
}

static uint32_t curCallNum = 0;
static uint32_t callNumWhereThreadsDisabled = 0;
void
ThreadMan_Lock(void)
{
    curCallNum++;

    if(!thread_lock)
        callNumWhereThreadsDisabled = curCallNum;

    thread_lock = TRUE;
}

void
ThreadMan_Unlock(void)
{
    if(callNumWhereThreadsDisabled == curCallNum--)
        thread_lock = FALSE;
}