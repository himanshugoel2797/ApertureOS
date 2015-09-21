#include "keyboard_proc.h"
#include "utils/common.h"

AOS_SCANCODES key_presses[KEYBOARD_BUFFER_SIZE];
uint32_t key_buf_write_pos;
uint32_t key_buf_read_pos;

static UID kbd_semaphore;

uint8_t
KeyboardProc_Initialize(void)
{
    memset(key_presses, 0, sizeof(uint16_t) * KEYBOARD_BUFFER_SIZE);
    key_buf_read_pos = 0;
    key_buf_write_pos = 0;

    kbd_semaphore = ThreadMan_CreateSemaphore(1);
}

void
KeyboardProc_WriteKey(AOS_SCANCODES scancode,
                      bool press)
{
            COM_WriteStr("Recieved!!!\r\n");
    if(ThreadMan_WaitAcquireSemaphore(kbd_semaphore))
        {
            key_presses[key_buf_write_pos++] = scancode | ((press & 1) << 31);
            if(key_buf_write_pos >= KEYBOARD_BUFFER_SIZE)key_buf_write_pos = 0;

            ThreadMan_ReleaseSemaphore(kbd_semaphore);
        }
}

AOS_SCANCODES
KeyboardProc_ReadKey(void)
{
    AOS_SCANCODES toRet = AP_NONE;
    if(ThreadMan_WaitAcquireSemaphore(kbd_semaphore))
        {
            if(key_buf_read_pos == key_buf_write_pos)toRet = AP_NONE;
            else toRet = key_presses[key_buf_read_pos++];

            if(key_buf_read_pos >= KEYBOARD_BUFFER_SIZE)key_buf_read_pos = 0;

            ThreadMan_ReleaseSemaphore(kbd_semaphore);
        }
    return toRet;
}