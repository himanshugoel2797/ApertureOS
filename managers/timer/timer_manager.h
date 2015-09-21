#ifndef _TIMER_MANAGER_H_
#define _TIMER_MANAGER_H_

#include "types.h"

#define MAX_TIMERS 2048
#define FREQ(x) (19886/x)

typedef void (*TickHandler)();

void Timers_Setup ();
UID Timers_CreateNew(uint32_t ticks, bool periodic, TickHandler handler);
void Timers_Delete(UID uid);
void Timers_StartTimer(UID uid);

#endif /* end of include guard: _TIMER_MANAGER_H_ */
