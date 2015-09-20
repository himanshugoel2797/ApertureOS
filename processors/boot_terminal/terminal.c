#include "terminal.h"
#include "utils/common.h"
#include "globals.h"

char *term_buffer = NULL;
UID term_buf_locked_id = 0;
uint32_t term_buffer_pos = 0;
uint32_t term_buf_len = 0;
uint32_t term_char_pitch = 0;
uint32_t term_char_rows = 0;
uint32_t term_draw_count = 0;

uint32_t
Terminal_Start(void)
{
    term_char_rows = global_multiboot_info->framebuffer_height / 16;
    term_char_pitch = global_multiboot_info->framebuffer_width / 8;
    term_buf_len = term_char_rows * term_char_pitch;
    term_buffer_pos = 0;

    term_buffer = kmalloc(term_buf_len);
    memset(term_buffer, 0, term_buf_len);

    UID kbd_thread = ThreadMan_CreateThread(Terminal_KeyboardThread, 0, NULL, THREAD_FLAGS_KERNEL);
    UID render_thread = ThreadMan_CreateThread(Terminal_DisplayThread, 0, NULL, THREAD_FLAGS_KERNEL);

    Terminal_Write("[himanshu@localhost]$", 21);

    ThreadMan_StartThread(kbd_thread);
    ThreadMan_StartThread(render_thread);

    while(1);
}

void
Terminal_Write(char *str,
               size_t len)
{
    //Keep yielding until the buffer is available
    if(term_buf_locked_id != ThreadMan_GetCurThreadID() && term_buf_locked_id != 0)
        {
            while(term_buf_locked_id != 0)
                ThreadMan_Yield();
        }

    term_buf_locked_id = ThreadMan_GetCurThreadID();

    if(term_buffer_pos + len >= term_buf_len)
        {
            memmove(term_buffer, term_buffer + len, term_buf_len - len);
            term_buffer_pos -= len;
        }
    memcpy(&term_buffer[term_buffer_pos], str, len);
    term_buffer_pos += len;
    term_buf_locked_id = 0;
}

void
Terminal_KeyboardThread(int argc,
                        char **argv)
{
    //Read input from the socket
    while(1)
    {

    }
}

void
Terminal_DisplayThread(int argc,
                       char **argv)
{
    while(1)
        {
            //Update the display based on the buffer
            graphics_Clear();

            if(term_buf_locked_id != ThreadMan_GetCurThreadID() && term_buf_locked_id != 0)
                {
                    while(term_buf_locked_id != 0)
                        ThreadMan_Yield();
                }

            term_buf_locked_id = ThreadMan_GetCurThreadID();

            //The buffer is locked for graphics stuff
            int x = 0, y = 0;
            for(int char_pos = 0; char_pos < term_buffer_pos; char_pos++)
                {
                    if(term_buffer[char_pos] < 0x20 || term_buffer[char_pos] > 0x7F)
                        {
                            if(term_buffer[char_pos] == 0x0A)y++;
                            if(term_buffer[char_pos] == 0x0D)x=0;
                            if(term_buffer[char_pos] == 0x00)break;
                            if(term_buffer[char_pos] == 0x08)
                            {
                                x--;
                                if(x < 0){
                                    y--;
                                    x = term_char_pitch - 1;
                                }
                            }
                            if(term_buffer[char_pos] == 0x09)x += (x % 4);
                            if(term_buffer[char_pos] == 0x0B)y += (y % 4);
                            if(term_buffer[char_pos] == 0x0C)
                            {
                                y = 0;
                                x = 0;
                                graphics_Clear();
                            }
                        }
                    else
                        {
                            char tmp_str[2] = {term_buffer[char_pos], 0};
                            graphics_Write(tmp_str, x * 8, y * 16);
                            x++;
                        }

                    if(x >= term_char_pitch)
                        {
                            y++;
                            x = 0;
                        }
                    if(y >= term_char_rows)break;

                }

            term_draw_count++;
            if(term_draw_count % 10 == 0)
            {
                term_draw_count = 0;
                graphics_WriteStr("_", x * 8, y * 16);
                graphics_WriteStr("_", x * 8, y * 16 - 1);
                graphics_WriteStr("_", x * 8, y * 16 - 2);
            }

            term_buf_locked_id = 0;
            graphics_SwapBuffer();
        }
}