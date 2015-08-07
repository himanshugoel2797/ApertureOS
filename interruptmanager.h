#ifndef _INTERRUPT_MANAGER_H_
#define _INTERRUPT_MANAGER_H_

#include <stddef.h>
#include <stdint.h>

#include "idt.h"

#define INTERRUPT_MANAGER_PRIORITY_COUNT 32
#define INTERRUPT_MANAGER_LOWEST_PRIORITY 30
#define INTERRUPT_MANAGER_HIGHEST_PRIORITY 0

void InterruptManager_Initialize();
void InterruptManager_RegisterHandler(uint8_t int_no, uint8_t priority, void (*handler)(Registers*));

#endif /* end of include guard: _INTERRUPT_MANAGER_H_ */
