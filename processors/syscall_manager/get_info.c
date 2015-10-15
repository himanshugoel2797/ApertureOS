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
                    syscall->retval = 0;
                    break;
                }
            break;
        }
    return;
}