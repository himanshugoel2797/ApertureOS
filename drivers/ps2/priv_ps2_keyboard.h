#ifndef _PRIV_PS2_KEYBOARD_H_
#define _PRIV_PS2_KEYBOARD_H_

#include "types.h"
#include "utils/keycodes.h"



enum {
        KBD_ENC_SET_LED = 0xED,
        KBD_ENC_ECHO = 0xEE,
        KBD_ENC_SET_SCANCODE_SET = 0xF0,
        KBD_ENC_GET_ID = 0xF2,
        KBD_ENC_SET_DELAY_AND_FREQ = 0xF3,
        KBD_ENC_ENABLE_KBD = 0xF4,
        KBD_ENC_RESET_DISABLE = 0xF5, //Reset and disable keyboard
        KBD_ENC_RESET_ENABLE = 0xF6,  //Reset and enable keyboard
        KBD_ENC_SET_ALL_AUTORPT = 0xF7,
        KBD_ENC_SET_ALL_MAKE_BREAK = 0xF8,
        KBD_ENC_SET_ALL_MAKE = 0xF9,
        KBD_ENC_SET_ALL_AUTORPT_MAKE_BREAK = 0xFA,
        KBD_ENC_SET_SINGLE_AUTORPT = 0xFB,
        KBD_ENC_SET_SINGLE_MAKE_BREAK = 0xFC,
        KBD_ENC_SET_SINGLE_BREAK = 0xFD,
        KBD_ENC_REPEAT = 0xFE,
        KBD_ENC_RESET_SELFTEST = 0xFD
};


#define SCROLL_LOCK_LED 0
#define NUM_LOCK_LED 2
#define CAPS_LOCK_LED 4
void PS2Keyboard_SetLEDs(uint8_t leds);

void PS2Keyboard_SetScancodeSet(uint8_t set_num);

#define DELAY_QUARTER_SECOND 0
#define DELAY_HALF_SECOND 1
#define DELAY_THREE_QUARTERS_SECOND 2
#define DELAY_ONE_SECOND 3
void PS2Keyboard_SetAutoRepeatRateAndDelay(uint8_t repeat_rate, uint8_t delay);


#endif /* end of include guard: _PRIV_PS2_KEYBOARD_H_ */
