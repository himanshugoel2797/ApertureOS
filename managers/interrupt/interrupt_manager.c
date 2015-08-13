#include "interrupt_manager.h"
#include "managers.h"
#include "drivers.h"

#include "utils/common.h"

SystemData *int_sys = NULL;
uint32_t interrupts_Initialize();
void interrupts_callback(uint32_t res);
uint8_t interrupts_messageHandler(Message *msg);

void interrupts_IDTHandler(Registers *Regs);

uint8_t using_apic = 0;
InterruptHandler int_handlers[INTERRUPT_COUNT][INTERRUPT_HANDLER_SLOTS + 1];

void Interrupts_Setup()
{
        int_sys = SysMan_RegisterSystem();
        strcpy(int_sys->sys_name, "interruptMan");
        int_sys->prerequisites[0] = 0;   //No prereqs
        int_sys->init = interrupts_Initialize;
        int_sys->init_cb = interrupts_callback;
        int_sys->msg_cb = interrupts_messageHandler;

        SysMan_StartSystem(int_sys->sys_id);
}

uint32_t interrupts_Initialize()
{
        int32_t pic_res = PIC_Initialize(); //Initialize the PIC
        int32_t apic_res = APIC_Initialize();  //Initialize the APIC

        if(apic_res < 0 && pic_res < 0) return -1;
        else if(apic_res < 0) using_apic = 0; //Using the PIC
        else if(apic_res == 0) using_apic = 1;  //Using the APIC

        memset(int_handlers, 0, sizeof(InterruptHandler) * INTERRUPT_COUNT * INTERRUPT_HANDLER_SLOTS);

        for(uint32_t i = 0; i < INTERRUPT_COUNT; i++)
        {
                IDT_RegisterHandler((uint8_t)i, interrupts_IDTHandler);
        }


        return 0;
}

void interrupts_IDTHandler(Registers *Regs)
{
        for(int i = 0; i < INTERRUPT_HANDLER_SLOTS + 1; i++)
        {
                if(int_handlers[Regs->int_no][i] != NULL)
                {
                        uint32_t res = int_handlers[Regs->int_no][i](Regs);
                        if(res) break;
                }
        }

        if(using_apic) APIC_SendEOI(Regs->int_no);
        else PIC_SendEOI(Regs->int_no);
}

void interrupts_callback(uint32_t res)
{
        if(res == -1) {

                Message msg;
                strcpy(msg.message, "Neither PIC nor APIC could be initialized");
                msg.system_id = int_sys->sys_id;
                msg.src_id = int_sys->sys_id;
                msg.msg_id = MI_INITIALIZATION_FAILURE;
                msg.msg_type = MT_ERROR;
                msg.msg_priority = MP_CRITICAL;

                MessageMan_Add(&msg);
        }
}

uint8_t interrupts_messageHandler(Message *msg)
{
        if(msg->msg_id == MI_INITIALIZATION_FAILURE) return 1;

        return 0;
}

void Interrupts_RegisterHandler(uint8_t intrpt, uint8_t slot, InterruptHandler handler)
{
        int_handlers[intrpt][slot] = handler;
}

uint8_t Interrupts_GetFreeSlot(uint8_t intrpt)
{
        uint8_t free_slot = 0;
        while(free_slot < INTERRUPT_HANDLER_SLOTS && int_handlers[intrpt][free_slot++] != NULL) ;
        return free_slot - 1;
}

void Interrupts_SetEnableMode(bool enabled)
{
        if(enabled) {
                asm volatile ("sti");
        }else{
                asm volatile ("cli");
        }
}

void Interrupts_SetInterruptMaskMode(uint8_t intrpt, bool masked)
{
        if(intrpt >= 32) {
                if(using_apic)
                {
                        //TODO setup interrupt masking on the IO APIC
                }else{
                        if(masked) PIC_MaskIRQ(intrpt);
                }
        }
}
