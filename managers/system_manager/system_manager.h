#ifndef _SYSTEM_MANAGER_H_
#define _SYSTEM_MANAGER_H_

#include "types.h"
#include "../msg_manager/msg_manager.h"
#include "../msg_manager/msg_types.h"

#define MAX_SYSTEM_COUNT 32
#define SYSTEM_NAME_LEN 16


typedef struct {
        char sys_name[SYSTEM_NAME_LEN];
        SysID prerequisites[MAX_SYSTEM_COUNT - 1];
        Initializer init;
        Callback init_cb;
        uint8_t(*msg_cb)(Message*);
        SysID sys_id;
}SystemData;

//Register a new system and assign it a UID
SystemData* SysMan_RegisterSystem();

//Register an initializer and initialization callback
void SysMan_StartSystem(SysID sys_id);

//Request a new UID
UID SysMan_RequestUID(SysID sys);

//Return UID
void SysMan_FreeUID(SysID sys, UID uid);

//Get data for a system, fill outData with 0xFF if the id is invalid
void SysMan_GetSystemData(SysID id, SystemData *outData);

#endif /* end of include guard: _SYSTEM_MANAGER_H_ */
