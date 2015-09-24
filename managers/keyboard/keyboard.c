#include "keyboard.h"
#include "managers.h"
#include "drivers.h"
#include "processors.h"
#include "scancodes.h"

static SystemData *kbd_sys = NULL;
static uint32_t kbd_Initialize();
static void kbd_callback(uint32_t res);
static uint8_t kbd_messageHandler(Message *msg);


static uint32_t Keyboard_ProcessInput(uint8_t input);
static void
Keyboard_PushInput(void);

static uint64_t keys_prev[8], keys_down[8];
static PS2_ScanCodes_2_ *scancodes_prev, *scancodes_down;

static uint8_t key_flags = 0;

void 
Keyboard_Setup(void)
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

static void 
keyboard_intHandler(Registers *regs)
{
    while(inb(0x64) & 1) Keyboard_ProcessInput(inb(0x60));
    Keyboard_PushInput();
}

static void sendKey(uint32_t val, uint64_t bmp, AOS_SCANCODES sc, bool down)
{
    if((bmp >> (uint64_t)((val - 1) % 64)) & 1)KeyboardProc_WriteKey(sc, down);
}

static void
Keyboard_PushInput(void)
{
    //Keyboard_ProcessInput(inb(0x60));
    if(key_flags & (1 << 2))
    {
        uint64_t diff[8];
        PS2_ScanCodes_2_ *diff_codes = (PS2_ScanCodes_2_*)diff;
        memset(diff, 0x00, sizeof(uint64_t) * 8);
        
        //determine which keys have changed
        for(int i = 0; i < 8; i++)diff[i] = keys_down[i];

        #define DIFF_PUSH(a, b) if((diff[(a - 1)/64] >> (uint64_t)((a - 1) % 64)) & 1)KeyboardProc_WriteKey(b, keys_down[(a-1)/64] >> (uint64_t)((a-1) % 64))
        #define EDIFF_PUSH(a,b) DIFF_PUSH(a, b)

        DIFF_PUSH(0x1C, AP_A);
        EDIFF_PUSH(0x32, AP_B);
        EDIFF_PUSH(0x21, AP_C);
        EDIFF_PUSH(0x23, AP_D);
        EDIFF_PUSH(0x24, AP_E);
        EDIFF_PUSH(0x2B, AP_F);
        EDIFF_PUSH(0x34, AP_G);
        EDIFF_PUSH(0x33, AP_H);
        EDIFF_PUSH(0x43, AP_I);
        EDIFF_PUSH(0x3B, AP_J);
        EDIFF_PUSH(0x42, AP_K);
        EDIFF_PUSH(0x4B, AP_L);
        EDIFF_PUSH(0x3A, AP_M);
        EDIFF_PUSH(0x31, AP_N);
        EDIFF_PUSH(0x44, AP_O);
        EDIFF_PUSH(0x4D, AP_P);
        EDIFF_PUSH(0x15, AP_Q);
        EDIFF_PUSH(0x2D, AP_R);
        EDIFF_PUSH(0x1B, AP_S);
        EDIFF_PUSH(0x2C, AP_T);
        EDIFF_PUSH(0x3C, AP_U);
        EDIFF_PUSH(0x2A, AP_V);
        EDIFF_PUSH(0x1D, AP_W);
        EDIFF_PUSH(0x22, AP_X);
        EDIFF_PUSH(0x35, AP_Y);
        EDIFF_PUSH(0x1A, AP_Z);
        EDIFF_PUSH(0x45, AP_0);
        EDIFF_PUSH(0x16, AP_1);
        EDIFF_PUSH(0x1E, AP_2);
        EDIFF_PUSH(0x26, AP_3);
        EDIFF_PUSH(0x25, AP_4);
        EDIFF_PUSH(0x2E, AP_5);
        EDIFF_PUSH(0x36, AP_6);
        EDIFF_PUSH(0x3D, AP_7);
        EDIFF_PUSH(0x3E, AP_8);
        EDIFF_PUSH(0x46, AP_9);
        EDIFF_PUSH(0x66, AP_BACKSPACE);
        EDIFF_PUSH(0x29, AP_SPACE);
        EDIFF_PUSH(0x5A, AP_ENTER);
        EDIFF_PUSH(0x175, AP_UP);
        EDIFF_PUSH(0x172, AP_DOWN);
        EDIFF_PUSH(0x16B, AP_LEFT);
        EDIFF_PUSH(0x174, AP_RIGHT);

        key_flags &= ~(1 << 2); //Unset the flag
        memcpy(keys_prev, keys_down, sizeof(uint64_t) * 8);
        memset(keys_down, 0, sizeof(uint64_t) * 8);
    }
}

static uint32_t 
Keyboard_ProcessInput(uint8_t input)
{
    if(input == 0xFA)return 0;

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


    //Check for certain scan codes (Lock keys_prev) to enable/disable the related LEDs
    if(input)
        {
            int64_t key_index = (input - 1) / 64;
            int64_t key_offset = (input - 1) % 64;

            if(key_flags & 1)
                key_index += 4;

            if((!(key_flags >> 1))) //Make code
                keys_down[key_index] |= (uint64_t)1 << (uint64_t)key_offset;
            else    //Break code
                keys_down[key_index] &= ~((uint64_t)1 << (uint64_t)key_offset);

            key_flags = 1 << 2; //Mark key input as present
        }
    return 0;

}



static uint32_t 
kbd_Initialize(void)
{
    memset(keys_prev, 0x00, sizeof(uint64_t) * 8);
    scancodes_prev = (PS2_ScanCodes_2_*)keys_prev;

    memset(keys_down, 0x00, sizeof(uint64_t) * 8);
    scancodes_down = (PS2_ScanCodes_2_*)keys_down;

    key_flags = 0;

    IOAPIC_MapIRQ(1, IRQ(1), APIC_GetID(), 0, 0, APIC_DELIVERY_MODE_FIXED);
    IOAPIC_SetEnableMode(IRQ(1), ENABLE);
    Interrupts_RegisterHandler(IRQ(1), 0, keyboard_intHandler);
    PS2_Initialize();
    PS2Keyboard_Initialize();
    UID kbd_timer = Timers_CreateNew(FREQ(12000), TRUE, Keyboard_PushInput);
    //Timers_StartTimer(kbd_timer);
}

static void 
kbd_callback(uint32_t res)
{

}


static uint8_t 
kbd_messageHandler(Message *msg)
{

}
