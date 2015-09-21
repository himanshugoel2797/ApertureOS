#include "keyboard_proc.h"
#include "utils/common.h"

uint16_t key_presses[KEYBOARD_BUFFER_SIZE];
uint32_t key_buf_write_pos;
uint32_t key_buf_read_pos;

uint8_t
KeyboardProc_Initialize(void)
{
	memset(key_presses, 0, sizeof(uint16_t) * KEYBOARD_BUFFER_SIZE);
	key_buf_read_pos = 0;
	key_buf_write_pos = 0;
}

void
KeyboardProc_WriteKey(AOS_SCANCODES scancode,
                      bool press)
{
	
}