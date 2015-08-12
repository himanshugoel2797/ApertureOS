#ifndef _MESSAGE_MANAGER_H_
#define _MESSAGE_MANAGER_H_

#include "types.h"
#include "../system_manager/system_manager.h"
#include "msg_types.h"

uint32_t MessageMan_Initialize();
void MessageMan_Add(Message *err);

#endif /* end of include guard: _MESSAGE_MANAGER_H_ */
