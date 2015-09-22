#include "timer_manager.h"
#include "managers.h"
#include "drivers.h"
#include "utils/common.h"

typedef struct
{
    uint32_t ticks;
    uint32_t curTicks;
    UID id, tid;
    bool periodic;
    TickHandler handler;
} TimerData;

static SystemData *timer_sys = NULL;
static uint32_t timers_Initialize();
static void timers_callback(uint32_t res);
static uint8_t timers_messageHandler(Message *msg);
static TimerData timer_entries[MAX_TIMERS];
static UID id_base = 0;
static uint64_t timer_ticks;


static void timer_handler(Registers *regs);
static void timer_callHandlers(int argc, char **argv);
static void timer_call(int argc, char **argv);

void Timers_Setup()
{
    timer_sys = SysMan_RegisterSystem();
    strcpy(timer_sys->sys_name, "timerMan");

    timer_sys->prerequisites[0] = Interrupts_GetSysID();
    timer_sys->prerequisites[1] = 0;

    timer_sys->init = timers_Initialize;
    timer_sys->init_cb = timers_callback;
    timer_sys->msg_cb = timers_messageHandler;

    SysMan_StartSystem(timer_sys->sys_id);
}

static uint32_t timers_Initialize()
{
    timer_ticks = 0;
    Interrupts_RegisterHandler(IRQ(0), 0, timer_handler);
    memset(timer_entries, 0, sizeof(timer_entries));
    //Initialize the PIT
    PIT_Initialize();
    PIT_SetEnableMode(DISABLE); //Disable it while we initialize everything else

    //Determine if the APIC is available and initialize its timer too
    if(Interrupts_IsAPICEnabled())
        {
            //TODO callibrate APIC timer
        }

    for(int i = 0; i < MAX_TIMERS; i++)
        {
            timer_entries[i].id = new_uid();
        }
    id_base = timer_entries[0].id;

    UID timer_thread = ThreadMan_CreateThread(timer_callHandlers, 0, NULL, THREAD_FLAGS_KERNEL);
    ThreadMan_StartThread(timer_thread);

    return 0;
}

static void timers_callback(uint32_t res)
{

}

static uint8_t timers_messageHandler(Message *msg)
{
    return -1;  //We shouldn't be recieving any messages
}

static void timer_call(int argc, char **argv)
{
    while(1){
        timer_entries[argc].handler();
        ThreadMan_SuspendThread(ThreadMan_GetCurThreadID());
        ThreadMan_Yield();
    }
    ThreadMan_DeleteThread(ThreadMan_GetCurThreadID());
}

static void timer_callHandlers(int argc, char **argv)
{
    while(1)
        {
            for(int i = 0; i < MAX_TIMERS; i++)
                {
                    if(timer_entries[i].ticks != 0 && timer_entries[i].curTicks == 0)
                        {
                            if(timer_entries[i].handler != NULL){
                                ThreadMan_StartThread(timer_entries[i].tid);
                            }
                            if(timer_entries[i].periodic)
                                {
                                    timer_entries[i].curTicks = timer_entries[i].ticks;
                                }
                            //COM_WriteStr("TEST!!!");
                        }
                }
        }
}

static void timer_handler(Registers *regs)
{
    timer_ticks++;
    for(int i = 0; i < MAX_TIMERS; i++)
        {
            if(timer_entries[i].ticks != 0)
                {
                    if(timer_entries[i].curTicks != 0)timer_entries[i].curTicks--;
                }
        }
}

UID Timers_CreateNew(uint32_t ticks, bool periodic, TickHandler handler)
{
    for(int i = 0; i < MAX_TIMERS; i++)
        {
            if(timer_entries[i].ticks == 0)
                {
                    timer_entries[i].ticks = ticks;
                    timer_entries[i].curTicks = ticks;
                    timer_entries[i].periodic = periodic;
                    timer_entries[i].handler = handler;
                    if(timer_entries[i].tid == 0)timer_entries[i].tid = ThreadMan_CreateThread(timer_call, i, NULL, THREAD_FLAGS_KERNEL);
                    return timer_entries[i].id;
                }
        }
    return -1;
}

void Timers_Delete(UID uid)
{
    uint32_t i = (uid - id_base);
    if(i < MAX_TIMERS)
        {
            timer_entries[i].ticks = 0;
            timer_entries[i].curTicks = 0;
        }
}

void Timers_StartTimer(UID uid)
{
    uint32_t i = (uid - id_base);
    if(i < MAX_TIMERS && timer_entries[i].ticks != 0)
        {
            PIT_SetEnableMode(ENABLE);
        }
}
