#ifndef _KEYBOARD_PROC_H_
#define _KEYBOARD_PROC_H_

#include "types.h"
#include "managers.h"

#define KEYBOARD_BUFFER_SIZE 256	//Store the last 256 key presses
#define KEY_RELEASE FALSE
#define KEY_PRESS TRUE

typedef enum
{
    AP_NONE = 0,
    AP_A = 1,
    AP_B = 2,
    AP_C = 3,
    AP_D = 4,
    AP_E = 5,
    AP_F = 6,
    AP_G = 7,
    AP_H = 8,
    AP_I = 9,
    AP_J = 10,
    AP_K = 11,
    AP_L = 12,
    AP_M = 13,
    AP_N = 14,
    AP_O = 15,
    AP_P = 16,
    AP_Q = 17,
    AP_R = 18,
    AP_S = 19,
    AP_T = 20,
    AP_U = 21,
    AP_V = 22,
    AP_W = 23,
    AP_X = 24,
    AP_Y = 25,
    AP_Z = 26,
    AP_L_SHIFT = 27,
    AP_R_SHIFT = 28,
    AP_SHIFT = 29,
    AP_L_CTRL = 30,
    AP_R_CTRL = 31,
    AP_CTRL = 32,
    AP_SPACE = 33,
    AP_SUPER = 34,
    AP_DELETE = 35,
    AP_BACKSPACE = 36,
    AP_ENTER = 37,
    AP_0 = 40 + 0,
    AP_1 = 40 + 1,
    AP_2 = 40 + 2,
    AP_3 = 40 + 3,
    AP_4 = 40 + 4,
    AP_5 = 40 + 5,
    AP_6 = 40 + 6,
    AP_7 = 40 + 7,
    AP_8 = 40 + 8,
    AP_9 = 40 + 9,
    AP_UP = 50,
    AP_DOWN = 51,
    AP_LEFT = 52,
    AP_RIGHT = 53,
    AP_FWD_SLASH = 54,
    AP_BKD_SLASH = 55

} AOS_SCANCODES;

uint8_t
KeyboardProc_Initialize(void);

void
KeyboardProc_WriteKey(AOS_SCANCODES scancode,
                      bool press);

AOS_SCANCODES
KeyboardProc_ReadKey(void);

#endif