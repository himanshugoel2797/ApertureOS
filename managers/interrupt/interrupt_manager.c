#include "interrupt_manager.h"
#include "managers.h"
#include "drivers.h"

#include "utils/common.h"

static SystemData *int_sys = NULL;
static uint32_t interrupts_Initialize(void);
static void interrupts_callback(uint32_t res);
static uint8_t interrupts_messageHandler(Message *msg);

static void interrupts_IDTHandler(Registers *Regs);
static void Interrupts_GPF_Handler(Registers *regs);

static uint8_t using_apic = 0;
static InterruptHandler int_handlers[INTERRUPT_COUNT][INTERRUPT_HANDLER_SLOTS + 1];
static int i2 = 0;
static bool inInterruptHandler;
static InterruptHandler unhandled_int;

void
Interrupts_Setup(void)
{
    int_sys = SysMan_RegisterSystem();
    strcpy(int_sys->sys_name, "interruptMan");
    int_sys->prerequisites[0] = 0;   //No prereqs
    int_sys->init = interrupts_Initialize;
    int_sys->init_cb = interrupts_callback;
    int_sys->msg_cb = interrupts_messageHandler;

    SysMan_StartSystem(int_sys->sys_id);
}

void
Interrupts_Virtualize(void)
{
    if(using_apic)
        {
            APIC_Virtualize();
            IOAPIC_VirtualizeAll();
        }
}

uint32_t
interrupts_Initialize(void)
{
    unhandled_int = NULL;
    inInterruptHandler = FALSE;
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

    for(int i = 16; i < 24; i++)IOAPIC_SetEnableMode(IRQ(i), ENABLE);

    //Register custom handler for GPF
    IDT_RegisterHandler(13, Interrupts_GPF_Handler);

    return 0;
}

void
interrupts_IDTHandler(Registers *Regs)
{
    bool handled = FALSE;
    inInterruptHandler = TRUE;
    for(int i = 0; i < INTERRUPT_HANDLER_SLOTS + 1; i++)
        {
            if(int_handlers[Regs->int_no][i] != NULL)
                {
                    handled = TRUE;
                    uint32_t res = int_handlers[Regs->int_no][i](Regs);
                    if(res) break;
                }
        }

    if(!handled)
        {
            if(unhandled_int != NULL)unhandled_int(Regs);   //Call the unhandled interrupt handler

            graphics_Write("Int#%x", 0, 1000, Regs->int_no);
            graphics_Write("#%x", 0, 1020, i2++);
            graphics_SwapBuffer();
            COM_WriteStr("Unhandled Int#%x\r\n", Regs->int_no);
        }

    if(using_apic) APIC_SendEOI(Regs->int_no);
    else PIC_SendEOI(Regs->int_no);
    inInterruptHandler = FALSE;
}

void
interrupts_callback(uint32_t res)
{
    if(res == -1)
        {

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

uint8_t
interrupts_messageHandler(Message *msg)
{
    if(msg->msg_id == MI_INITIALIZATION_FAILURE) return 1;

    return 0;
}

void
Interrupts_RegisterHandler(uint8_t intrpt,
                           uint8_t slot,
                           InterruptHandler handler)
{
    int_handlers[intrpt][slot] = handler;
}

void
Interrupts_RaiseOnUnhandled(InterruptHandler handler)
{
    unhandled_int = handler;
}

uint8_t
Interrupts_GetFreeSlot(uint8_t intrpt)
{
    uint8_t free_slot = 0;
    while(free_slot < INTERRUPT_HANDLER_SLOTS && int_handlers[intrpt][free_slot++] != NULL) ;
    return free_slot - 1;
}

void
Interrupts_EmptySlot(uint8_t intrpt,
                     uint8_t slot)
{
    if(slot < INTERRUPT_HANDLER_SLOTS) int_handlers[intrpt][slot] = NULL;
}

void
Interrupts_GetHandler(uint8_t intrpt,
                      uint8_t slot,
                      InterruptHandler* o_handler)
{
    if(slot < INTERRUPT_HANDLER_SLOTS)
        {
            *o_handler = int_handlers[intrpt][slot];
        }
}

void
Interrupts_SetInterruptEnableMode(uint8_t intrpt,
                                  bool enabled)
{
    if(intrpt >= 32)
        {
            if(using_apic)
                {
                    IOAPIC_SetEnableMode(intrpt, enabled);
                }
            else
                {
                    if(!enabled) PIC_MaskIRQ(intrpt);
                    else PIC_UnMaskIRQ(intrpt);
                }
        }
}

SysID Interrupts_GetSysID(void)
{
    return int_sys->sys_id;
}

bool Interrupts_IsAPICEnabled(void)
{
    return using_apic;
}

static uint32_t curCallNum = 0;
static uint32_t callNumWhereIntsEnabled = 0;
void
Interrupts_Lock(void)
{
    uint16_t flags = 0;
    curCallNum++;

    asm volatile ("pushf\n\tpop %%eax" : "=a" (flags));
    if((flags & (1<<9)) == (1<<9))   //Check if interrupts are enabled
        {
            callNumWhereIntsEnabled = curCallNum;
        }

    asm volatile ("cli");
}

void
Interrupts_Unlock(void)
{
    if(callNumWhereIntsEnabled == curCallNum--)
        {
            asm volatile ("sti");
        }
}

void
Interrupts_GPF_Handler(Registers *regs)
{
    COM_WriteStr("General Protection Fault (%x): Register Dump\r\n", regs->err_code);
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
    COM_WriteStr("EBP: %x\t", regs->ebp);
    asm volatile("hlt");
}

bool
Interrupts_IsInHandler(void)
{
    return inInterruptHandler;
}