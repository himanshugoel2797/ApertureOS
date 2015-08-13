#ifndef _TIMER_MANAGER_H_
#define _TIMER_MANAGER_H_

#include "types.h"

typedef void (*TickHandler)();

void Timers_Setup ();
UID Timers_CreateNew(uint32_t frequency, TickHandler handler);
void Timers_Delete(UID uid);

#endif /* end of include guard: _TIMER_MANAGER_H_ */
