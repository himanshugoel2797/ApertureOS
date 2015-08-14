#include "timer_manager.h"
#include "managers.h"
#include "drivers.h"
#include "utils/common.h"

typedef struct {
        uint32_t ticks;
        uint32_t curTicks;
        bool periodic;
        TickHandler handler;
}TimerData;

SystemData *timer_sys = NULL;
uint32_t timers_Initialize();
void timers_callback(uint32_t res);
uint8_t timers_messageHandler(Message *msg);
TimerData timer_entries[MAX_TIMERS];


void timer_handler(Registers *regs);

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

uint32_t timers_Initialize()
{
        //Initialize the PIT
        PIT_Initialize();
        PIT_SetEnableMode(DISABLE); //Disable it while we initialize everything else

        Interrupts_RegisterHandler(IRQ(0), 0, timer_handler);
        memset(timer_entries, 0, sizeof(timer_entries));

        //Determine if the APIC is available and initialize its timer too
        if(Interrupts_IsAPICEnabled())
        {
                //TODO callibrate APIC timer
        }


        //APIC_SetTimerMode(APIC_TIMER_PERIODIC);

        //APIC_SetTimerValue(1 << 25);

        //APIC_SetVector(APIC_TIMER, 34);
        //APIC_SetEnableInterrupt(APIC_TIMER, 1);

        return 0;
}

void timers_callback(uint32_t res)
{

}

uint8_t timers_messageHandler(Message *msg)
{
        return -1;  //We hould't be recieving any messages
}

void timer_handler(Registers *regs)
{
        for(int i = 0; i < MAX_TIMERS; i++)
        {
                if(timer_entries[i].ticks != 0)
                {
                        timer_entries[i].curTicks--;
                        COM_WriteStr("%d\r\n", timer_entries[i].curTicks);
                        if(timer_entries[i].curTicks == 0)
                        {
                                if(timer_entries[i].handler != NULL) timer_entries[i].handler();
                                if(timer_entries[i].periodic)
                                {
                                        timer_entries[i].curTicks = timer_entries[i].ticks;
                                }
                        }
                }
        }
}

UID Timers_CreateNew(uint32_t ticks, bool periodic, TickHandler handler)
{
        UID id = 0x80000000;
        for(int i = 0; i < MAX_TIMERS; i++)
        {
                if(timer_entries[i].ticks == 0)
                {
                        id += i;
                        timer_entries[i].ticks = ticks;
                        timer_entries[i].curTicks = ticks;
                        timer_entries[i].periodic = periodic;
                        timer_entries[i].handler = handler;
                        return id;
                }
        }
        return -1;
}

void Timers_Delete(UID uid)
{
        uint32_t i = (uid - 0x80000000);
        if(i < MAX_TIMERS)
        {
                timer_entries[i].ticks = 0;
                timer_entries[i].curTicks = 0;
        }
}

void Timers_StartTimer(UID uid)
{
        uint32_t i = (uid - 0x80000000);
        if(i < MAX_TIMERS && timer_entries[i].ticks != 0)
        {
                PIT_SetEnableMode(ENABLE);
        }
}
