#include "keyboard_proc.h"
#include "utils/common.h"

AOS_SCANCODES key_presses[KEYBOARD_BUFFER_SIZE];
int32_t key_buf_write_pos;
int32_t key_buf_read_pos;
bool key_avail;

static UID kbd_semaphore;

uint8_t
KeyboardProc_Initialize(void)
{
    memset(key_presses, 0, sizeof(uint16_t) * KEYBOARD_BUFFER_SIZE);
    key_buf_read_pos = 0;
    key_buf_write_pos = 0;
    key_avail = FALSE;

    kbd_semaphore = ThreadMan_CreateSemaphore(1);
}

void
KeyboardProc_WriteKey(AOS_SCANCODES scancode,
                      bool press)
{
    uint32_t w_pos = key_buf_write_pos;
    key_presses[w_pos++] = scancode | ((!press & 1) << 31);
    //COM_WriteStr("Keypress: %x\r\n", key_presses[w_pos - 1]);

    if(w_pos >= KEYBOARD_BUFFER_SIZE)
        {
            w_pos = 0;
            key_buf_read_pos = KEYBOARD_BUFFER_SIZE - 1;
            memset(key_presses, 0, (KEYBOARD_BUFFER_SIZE - 1) * sizeof(AOS_SCANCODES));
        }

    key_buf_write_pos = w_pos;
}

AOS_SCANCODES
KeyboardProc_ReadKey(void)
{
    if(key_buf_read_pos >= KEYBOARD_BUFFER_SIZE)key_buf_read_pos = -1;
    if(key_presses[key_buf_read_pos] != AP_NONE)
        {
            //COM_WriteStr("Scancode: %x\r\n", key_presses[key_buf_read_pos]);
            key_buf_read_pos++;
            return key_presses[key_buf_read_pos - 1];
        }
    return AP_NONE;
}