#include "keyboard.h"
#include "managers.h"
#include "drivers.h"


SystemData *kbd_sys = NULL;
uint32_t kbd_Initialize();
void kbd_callback(uint32_t res);
uint8_t kbd_messageHandler(Message *msg);

void Keyboard_Setup()
{
        kbd_sys = SysMan_RegisterSystem();
        strcpy(kbd_sys->sys_name, "keyboardMan");

        kbd_sys->prerequisites[0] = Interrupts_GetSysID();
        kbd_sys->prerequisites[1] = 0;

        kbd_sys->init = kbd_Initialize;
        kbd_sys->init_cb = kbd_callback;
        kbd_sys->msg_cb = kbd_messageHandler;

        SysMan_StartSystem(kbd_sys->sys_id);
}

void keyboard_test(Registers *regs)
{
        COM_WriteStr("Keyboard Input Recieved!\r\n");
        //temp2 = inb(0x60);

        while(inb(0x64) & 1) inb(0x60);

        //allocLoc++;
}

uint32_t kbd_Initialize()
{
        IOAPIC_MapIRQ(1, IRQ(1), APIC_GetID(), 0, 0, APIC_DELIVERY_MODE_FIXED);
        IOAPIC_SetEnableMode(IRQ(1), ENABLE);
        Interrupts_RegisterHandler(IRQ(1), 0, keyboard_test);
        PS2_Initialize();
        PS2Keyboard_Initialize();
}

void kbd_callback(uint32_t res)
{

}


uint8_t kbd_messageHandler(Message *msg)
{

}
