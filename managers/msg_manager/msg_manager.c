#include "msg_manager.h"

#include "utils/common.h"
#include "drivers/drivers.h"

void MessageMan_Add(Message *err)
{
        if(err == NULL) return; //Ignore all invalid pointers

        SystemData sys;
        SysMan_GetSystemData(err->system_id, &sys);

        char *sys_n = "Unknown";

        if(sys.sys_id != err->system_id) {
                COM_WriteStr("SEVERE WARNING [MessageMan]: Invalid System ID for message %s\r\n", err->message);
        }else{
                sys_n = sys.sys_name;
        }

        COM_WriteStr("PRIORITY %d, STATUS %d, ID %d [%s]: %s\r\n",
                     err->msg_priority, err->msg_type, err->msg_id, sys_n, err->message);

        //If this is a valid system ID, call the message handler if available
        if(sys.sys_id == err->system_id)
        {
                if(sys.msg_cb != NULL) {
                        uint8_t res = sys.msg_cb(err);
                        if(res == 0) {
                                COM_WriteStr("[%s] Situation Solved\r\n", sys_n);
                        }else{
                                COM_WriteStr("[%s] Failed to resolve situation! [CODE %d]\r\n", sys_n, res);
                        }
                }else if(err->msg_priority != MP_CRITICAL) {
                        COM_WriteStr("SEVERE WARNING [MessageMan]: System %s does not have a message callback setup, messages will be ignored\r\n", sys_n);
                }else{
                        COM_WriteStr("%s System with ID%d has a critical message waiting, this message must be handled, execution will be stopped\r\n", sys_n, err->system_id);
                        while(1);
                }
        }
        else if(err->msg_priority == MP_CRITICAL) {
                COM_WriteStr("Unknown System with ID%d has a critical message waiting, this message must be handled, execution will be stopped", err->system_id);
                while(1);
        }
}
