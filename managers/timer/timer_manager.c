#include "timer_manager.h"
#include "managers.h"

SystemData *timer_sys = NULL;
uint32_t timers_Initialize();
void timers_callback(uint32_t res);
uint8_t timers_messageHandler(Message *msg);

void Timers_Setup()
{
        timer_sys = SysMan_RegisterSystem();
        strcpy(timer_sys->sys_name, "timerMan");

        timer_sys->prerequisites[0] = 0;  //No prerequisites

        timer_sys->init = timers_Initialize;
        timer_sys->init_cb = timers_callback;
        timer_sys->msg_cb = timers_messageHandler;

        SysMan_StartSystem(timer_sys->sys_id);
}

uint32_t timers_Initialize()
{
      //Initialize the PIT

      //Determine if the APIC is available and initialize its timer too
}

void timers_callback(uint32_t res)
{

}

uint8_t timers_messageHandler(Message *msg)
{

}
