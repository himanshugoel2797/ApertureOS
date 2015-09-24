#include "terminal.h"
#include "utils/common.h"
#include "globals.h"
#include "keyboard_proc/keyboard_proc.h"

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

    UID kbd_thread = Timers_CreateNew(FREQ(60), TRUE, Terminal_KeyboardThread);
    UID render_thread = Timers_CreateNew(FREQ(60), TRUE, Terminal_DisplayThread);

    Terminal_Write("[user@localhost /]#", 17);

    //Timers_StartTimer(kbd_thread);
    Timers_StartTimer(render_thread);

    ThreadMan_SuspendThread(ThreadMan_GetCurThreadID());
}

void
Terminal_Write(char *str,
               size_t len)
{
    //Keep yielding until the buffer is available
    //if(term_buf_locked_id != ThreadMan_GetCurThreadID() && term_buf_locked_id != 0)
    //    {
    //        while(term_buf_locked_id != 0)
    //            ThreadMan_Yield();
    //    }

    //term_buf_locked_id = ThreadMan_GetCurThreadID();

    if(term_buffer_pos + len >= term_buf_len)
        {
            memmove(term_buffer, term_buffer + len, term_buf_len - len);
            term_buffer_pos -= len;
        }
    memcpy(&term_buffer[term_buffer_pos], str, len);
    term_buffer_pos += len;
    term_buf_locked_id = 0;
    COM_WriteStr(str);
}

char*
Terminal_GetPrevLine(void)
{
    int32_t i;
    for(i = term_buffer_pos; i >= 0; i--)
        {
            if(term_buffer[i] == '\n' && term_buffer[i - 1] == '\r')break;
        }
    return &term_buffer[i];
}

void
Terminal_ExecuteCmd(const char *cmd)
{
    if(strncmp(cmd, "ls", 2) == 0)Terminal_Write("\r\nList Dir", 10);
}

void
Terminal_KeyboardThread(void)
{
    //Read input from the socket
    AOS_SCANCODES key = KeyboardProc_ReadKey();
    //COM_WriteStr("Poll!!!!\r\n");
    if(key == AP_A)Terminal_Write("a", 1);
    else if(key == AP_B)Terminal_Write("b", 1);
    else if(key == AP_C)Terminal_Write("c", 1);
    else if(key == AP_D)Terminal_Write("d", 1);
    else if(key == AP_E)Terminal_Write("e", 1);
    else if(key == AP_F)Terminal_Write("f", 1);
    else if(key == AP_G)Terminal_Write("g", 1);
    else if(key == AP_H)Terminal_Write("h", 1);
    else if(key == AP_I)Terminal_Write("i", 1);
    else if(key == AP_J)Terminal_Write("j", 1);
    else if(key == AP_K)Terminal_Write("k", 1);
    else if(key == AP_L)Terminal_Write("l", 1);
    else if(key == AP_M)Terminal_Write("m", 1);
    else if(key == AP_N)Terminal_Write("n", 1);
    else if(key == AP_O)Terminal_Write("o", 1);
    else if(key == AP_P)Terminal_Write("p", 1);
    else if(key == AP_Q)Terminal_Write("q", 1);
    else if(key == AP_R)Terminal_Write("r", 1);
    else if(key == AP_S)Terminal_Write("s", 1);
    else if(key == AP_T)Terminal_Write("t", 1);
    else if(key == AP_U)Terminal_Write("u", 1);
    else if(key == AP_V)Terminal_Write("v", 1);
    else if(key == AP_W)Terminal_Write("w", 1);
    else if(key == AP_X)Terminal_Write("x", 1);
    else if(key == AP_Y)Terminal_Write("y", 1);
    else if(key == AP_Z)Terminal_Write("z", 1);

    else if(key == AP_0)Terminal_Write("0", 1);
    else if(key == AP_1)Terminal_Write("1", 1);
    else if(key == AP_2)Terminal_Write("2", 1);
    else if(key == AP_3)Terminal_Write("3", 1);
    else if(key == AP_4)Terminal_Write("4", 1);
    else if(key == AP_5)Terminal_Write("5", 1);
    else if(key == AP_6)Terminal_Write("6", 1);
    else if(key == AP_7)Terminal_Write("7", 1);
    else if(key == AP_8)Terminal_Write("8", 1);
    else if(key == AP_9)Terminal_Write("9", 1);

    else if(key == AP_SPACE)Terminal_Write(" ", 1);
    else if(key == AP_BACKSPACE)Terminal_Write("\b", 1);
    else if(key == AP_ENTER)
        {
            //Read the command and pass it on to the command parser
            char *prevLine = Terminal_GetPrevLine();
            if(prevLine != NULL)
                {
                    Terminal_ExecuteCmd(prevLine + 18);
                }

            Terminal_Write("\r\n", 2);
            Terminal_Write("[user@localhost]#", 17);
        }
    else if(key == AP_UP)term_buffer_pos -= term_char_pitch;
    else if(key == AP_DOWN)term_buffer_pos += term_char_pitch;
    else if(key == AP_RIGHT)term_buffer_pos++;
    else if(key == AP_LEFT)term_buffer_pos--;

}

void
Terminal_DisplayThread(void)
{
    //Update the display based on the buffer
    //graphics_Clear();

    //if(term_buf_locked_id != ThreadMan_GetCurThreadID() && term_buf_locked_id != 0)
    //    {
    //        while(term_buf_locked_id != 0);
    //    }

    //term_buf_locked_id = ThreadMan_GetCurThreadID();
    Terminal_KeyboardThread();
    //The buffer is locked for graphics stuff
    int x = 0, y = 0;
    for(int char_pos = 0; char_pos < term_buf_len; char_pos++)
        {
            if(term_buffer[char_pos] < 0x20 || term_buffer[char_pos] > 0x7F)
                {
                    if(term_buffer[char_pos] == 0x0A)y++;
                    if(term_buffer[char_pos] == 0x0D)x=0;
                    if(term_buffer[char_pos] == 0x00)break;
                    if(term_buffer[char_pos] == 0x08)
                        {
                            term_buffer[char_pos - 1] = 1;
                            term_buffer[char_pos] = 1;
                            term_buffer_pos -=2;
                            x--;
                            if(x < 0)
                                {
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
    if(term_draw_count % 10 > 3)
        {
            term_draw_count = 0;
            graphics_WriteStr("_", x * 8, y * 16);
            graphics_WriteStr("_", x * 8, y * 16 - 1);
            graphics_WriteStr("_", x * 8, y * 16 - 2);
        }

    term_buf_locked_id = 0;
    graphics_SwapBuffer();
}