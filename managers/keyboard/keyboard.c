#include "keyboard.h"
#include "managers.h"
#include "drivers.h"
#include "scancodes.h"

SystemData *kbd_sys = NULL;
uint32_t kbd_Initialize();
void kbd_callback(uint32_t res);
uint8_t kbd_messageHandler(Message *msg);


uint32_t Keyboard_ProcessInput(uint8_t input);

uint64_t keys[8];
PS2_ScanCodes_2_ *scancodes;

uint8_t key_flags = 0;

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
    //COM_WriteStr("Keyboard Input Recieved! %x\r\n");
    Keyboard_ProcessInput(inb(0x60));

    while(inb(0x64) & 1) inb(0x60);
    memset(keys, 0, sizeof(uint64_t) * 8);
}


uint32_t Keyboard_ProcessInput(uint8_t input)
{
    //Set the flag for the key down

    //An extended key code is expected
    if(input == 0xE0)
        {
            key_flags |= 1;
            return 0;
        }

    //Break code
    if(input == 0xF0)
        {
            key_flags |= 2;
            return 0;
        }


    //Check for certain scan codes (Lock keys) to enable/disable the related LEDs
    if(input)
        {
            int64_t key_index = (input - 1) / 64;
            int64_t key_offset = (input - 1) % 64;

            if(key_flags & 1)
                {
                    key_index += 4;
                }

            //keys[key_index] = SET_VAL_BIT(keys[key_index], key_offset, ((~key_flags & 2) >> 1)  );
            COM_WriteStr("Key Press: %x, %d, %d\r\n %b \r\n", input, key_index, key_offset, keys[key_index]);
            if((!(key_flags >> 1)))
                {
                    if(input == 0x7E)PS2Keyboard_SetLEDStatus(1, 1);
                    if(input == 0x58)PS2Keyboard_SetLEDStatus(2, 1);
                    COM_WriteStr(" Make!\r\n");

                    keys[key_index] |= (uint64_t)1 << (uint64_t)key_offset;
                }
            else
                {
                    if(input == 0x7E)PS2Keyboard_SetLEDStatus(1, 0);
                    if(input == 0x58)PS2Keyboard_SetLEDStatus(2, 0);
                    COM_WriteStr(" Break!\r\n");

                    keys[key_index] &= ~((uint64_t)1 << (uint64_t)key_offset);
                }

            key_flags = 0;
        }
    return 0;

}

uint32_t kbd_Initialize()
{
    memset(keys, 0, sizeof(uint64_t) * 8);
    scancodes = (PS2_ScanCodes_2_*)keys;

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
