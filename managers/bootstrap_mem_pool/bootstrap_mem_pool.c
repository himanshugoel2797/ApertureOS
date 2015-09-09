#include "bootstrap_mem_pool.h"
#include "managers.h"

uint8_t mem_pool[BOOTSTRAP_MEM_POOL];
uint32_t pos = 0;
SystemData *btstrp_sys = NULL;

uint32_t bootstrap_Initialize();
void bootstrap_callback(uint32_t res);
uint8_t bootstrap_messageHandler(Message *msg);

void bootstrap_setup()
{
    btstrp_sys = SysMan_RegisterSystem();
    strcpy(btstrp_sys->sys_name, "bootstrapMan");
    btstrp_sys->prerequisites[0] = 0; //No prereqs
    btstrp_sys->init = bootstrap_Initialize;
    btstrp_sys->init_cb = bootstrap_callback;
    btstrp_sys->msg_cb = bootstrap_messageHandler;

    SysMan_StartSystem(btstrp_sys->sys_id);
}

uint32_t bootstrap_Initialize()
{
    return 0;
}

void bootstrap_callback(uint32_t res)
{
    if(!res) return;
}

uint8_t bootstrap_messageHandler(Message *msg)
{
    //Fail on critical errors
    if(msg->msg_priority == MP_CRITICAL) return 1;
    else return 0;
}

void* bootstrap_malloc(size_t size)
{
    //Make sure size is 32bit aligned
    size += (size % 4);
    if( (pos + size) > BOOTSTRAP_MEM_POOL )
        {
            Message msg;
            strcpy(msg.message, "Not enough space for allocation");
            msg.system_id = btstrp_sys->sys_id;
            msg.src_id = btstrp_sys->sys_id;
            msg.msg_id = MI_OUT_OF_MEMORY;
            msg.msg_type = MT_ERROR;
            msg.msg_priority = MP_CRITICAL;

            MessageMan_Add(&msg);

            return NULL;
        }

    void *ptr = &mem_pool[pos];
    pos += size;
    return ptr;
}
