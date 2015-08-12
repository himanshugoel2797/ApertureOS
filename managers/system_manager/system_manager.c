#include "system_manager.h"
#include "priv_sys_man.h"

#include "utils/common.h"

SystemData systems[MAX_SYSTEM_COUNT];
uint8_t sys_started[MAX_SYSTEM_COUNT];
SysID curSID = 1;

UID_LList uids; //Allocate next uid list at 1 before the end so we have room

SystemData* SysMan_RegisterSystem()
{
        if( (curSID - 1) < MAX_SYSTEM_COUNT) {
                memset(&systems[curSID - 1], 0, sizeof(SystemData));
                systems[curSID - 1].sys_id = curSID;
                sys_started[curSID - 1] = 0;
                curSID++;
                return &systems[curSID - 2];
        }
        else
        {
          
                return NULL;
        }
}

void SysMan_GetSystemData(SysID id, SystemData *outData)
{
        if(outData != NULL && id < curSID)
        {
                memcpy(outData, &systems[id - 1], sizeof(SystemData));
        }
        else if(outData != NULL)
        {
                memset(outData, -1, sizeof(SystemData));
        }
}

void SysMan_StartSystem(SysID sys_id)
{
        //Check if all prereqs of specified system are met
        SysID i = 0, total = 0, prev = -1;
        sys_id--;

        sys_started[sys_id] = 1;  //Mark this system as started

        for(; systems[sys_id].prerequisites[i] && i < (MAX_SYSTEM_COUNT - 1); i++)
        {
                SysID s = systems[sys_id].prerequisites[i] - 1;
                if(sys_started[s] && systems[s].init_cb == NULL) total++;
                else if(sys_started[s]) {
                        if(s == prev) break; //We're checking this for the second time, initialization for the system failed, so current system can't be initialized either
                        SysMan_StartSystem(s + 1);
                        prev = s;
                        i--;  //Evaluate this one again to see if it has been activated
                }
                else break; //If a system hasn't been initialized yet, break
        }
        if(total == i) {
                uint32_t ret = systems[sys_id].init();

                systems[sys_id].init_cb(ret);
                systems[sys_id].init_cb = NULL;
        }
}
