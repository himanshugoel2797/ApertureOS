#include "ps2_keyboard.h"
#include "priv_ps2.h"
#include "priv_ps2_keyboard.h"

#include "utils/native.h"
#include "utils/keycodes.h"

uint8_t PS2Keyboard_Initialize()
{
        //Reset and enable keyboard
        uint8_t bat_status = 0;
        PS2_SendCommand(PS2_DATA_PORT, KBD_ENC_RESET_ENABLE, &bat_status);
        uint8_t ack = PS2_ReadData(PS2_DATA_PORT);
        if( (bat_status != PS2_RETVAL_BAT_SUCCESS) | (ack != PS2_RETVAL_ACK)) return -1;

        PS2Keyboard_SetScancodeSet(2);

        return 0;
}


void PS2Keyboard_SetLEDs(uint8_t leds)
{
        leds &= 0x7;  //Mask out all unused bits
        outb(PS2_DATA_PORT, KBD_ENC_SET_LED);
        outb(PS2_DATA_PORT, leds);
}

void PS2Keyboard_SetScancodeSet(uint8_t set_num)
{
        if(set_num == 0 || set_num > 3) return;
        uint8_t set = 1 << set_num;

        while(IS_OUTPUTBUF_FULL) ;
        outb(PS2_DATA_PORT, KBD_ENC_SET_SCANCODE_SET);
        while(IS_OUTPUTBUF_FULL) ;
        outb(PS2_DATA_PORT, set);
}

void PS2Keyboard_SetAutoRepeatRateAndDelay(uint8_t repeat_rate, uint8_t delay)
{
        uint8_t data = 0;
        data = (30 - repeat_rate) & 0x0F; //Approximation
        data |= (delay & 3) << 5;

        outb(PS2_DATA_PORT, KBD_ENC_SET_DELAY_AND_FREQ);
        outb(PS2_DATA_PORT, data);
}
