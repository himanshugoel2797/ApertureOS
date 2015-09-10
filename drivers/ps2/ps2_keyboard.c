#include "ps2_keyboard.h"
#include "priv_ps2.h"
#include "utils/native.h"

uint8_t PS2Keyboard_Initialize()
{
    //Reset the keyboard
    WAIT_DATA_SENT;
    outb(DATA_PORT, 0xFF);
    WAIT_DATA_AVL;
    uint16_t reset_res = 0;
    inb(DATA_PORT);
    WAIT_DATA_AVL;
    inb(DATA_PORT);

    outb(DATA_PORT, 0xF8);	//Enable make and break codes for all keys
    inb(DATA_PORT);
    inb(DATA_PORT);
}
