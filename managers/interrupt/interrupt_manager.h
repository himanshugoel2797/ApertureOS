#ifndef _INTERRUPT_MANAGER_H_
#define _INTERRUPT_MANAGER_H_

#include "types.h"
#include "idt.h"

typedef uint32_t (*InterruptHandler)(Registers*);

#define INTERRUPT_HANDLER_SLOTS 128
#define INTERRUPT_COUNT IDT_ENTRY_COUNT
#define IRQ(n) (n + 32)
#define INTR(n) (n)

void
Interrupts_Setup(void);

void
Interrupts_Virtualize(void);

void
Interrupts_RegisterHandler(uint8_t intrpt,
                           uint8_t slot,
                           InterruptHandler handler);

uint8_t
Interrupts_GetFreeSlot(uint8_t intrpt);

void
Interrupts_EmptySlot(uint8_t intrpt,
                     uint8_t slot);

void
Interrupts_GetHandler(uint8_t intrpt,
                      uint8_t slot,
                      InterruptHandler* o_handler);

void
Interrupts_SetInterruptEnableMode(uint8_t intrpt,
                                  bool masked);

SysID
Interrupts_GetSysID(void);

bool
Interrupts_IsAPICEnabled(void);

bool
Interrupts_IsInHandler(void);

void
Interrupts_Lock(void);

void
Interrupts_Unlock(void);

#endif /* end of include guard: _INTERRUPT_MANAGER_H_ */
