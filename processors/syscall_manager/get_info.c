#include "syscall_man.h"
#include "graphics/graphics.h"
#include "syscalls.h"

void
syscall_GetSysInfo(void *args)
{
    generic_syscall_3 *syscall = (generic_syscall_3*)args;

    syscall->retval = -1;

    COM_WriteStr("SYSCALL!!!!\r\n");

    switch(syscall->arg0)
        {
        case APEROS_PROC_DATA:
            switch(syscall->arg1)
                {
                case APEROS_PROC_ID:
                    syscall->retval = ProcessManager_GetCurPID();
                    break;
                case APEROS_PROC_DATA_ARGC:
                    syscall->retval = 1;
                    break;
                case APEROS_PROC_DATA_ARGV:
                {
                    uint8_t* argv = (uint8_t*)0x40004000;
                    strcpy(argv, "test2");
                    syscall->retval = 0x40004000;   //The argv data is always placed right after the stack
                }
                break;
                }
            break;
        case APEROS_ENV_DATA:
            switch(syscall->arg1)
                {
                case APEROS_DISPLAY_INFO:
                    RET_CHECK_PRIV_ADDR(syscall->arg2);
                    DisplayInfo *disp_info = (DisplayInfo*)syscall->arg2;

                    COM_WriteStr("DisplayInfo param: %d\r\n", disp_info->size);
                    if(disp_info->size > sizeof(DisplayInfo))return;

                    memcpy(disp_info, graphics_GetDisplayInfoPtr(), disp_info->size);
                    if( (ProcessManager_GetCurProcessInfo()->flags & 1) == 0)
                        {

                            disp_info->framebuffer_addr = NULL;
                            disp_info->backbuffer_addr = NULL;
                        }
                    syscall->retval = 0;
                    break;
                case APEROS_ENV_VAR:
                    syscall->retval = NULL;
                    break;
                }
            break;
        }
    return;
}