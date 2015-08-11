#include "msg_manager.h"

Message msg_pool[ERROR_POOL_SIZE];
uint32_t cur_msg_index = 0;

void Message_Add(Message *err)
{
        if(err->msg_priority == MP_CRITICAL)
        {
                //Immediately deal with critical messages
        }else{  //Other messages can wait
                strcpy(msg_pool[cur_error_index].message, err->message);
                msg_pool[cur_msg_index].system_id = err->system_id;
                msg_pool[cur_msg_index].stack_pointer = err->stack_pointer;
                msg_pool[cur_msg_index].msg_id = err->msg_id;
                msg_pool[cur_msg_index].msg_type = err->msg_type;
                msg_pool[cur_msg_index].msg_priority = err->msg_priority;

                cur_msg_index++;
        }
}

void Error_HandleErrors()
{

}
