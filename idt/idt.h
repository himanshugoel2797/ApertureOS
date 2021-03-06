#ifndef _IDT_H_
#define _IDT_H_

#include "types.h"

#define IDT_ENTRY_COUNT 256
#define IDT_ENTRY_HANDLER_SIZE 0x30


typedef struct
{
    uint32_t ds;             // Data segment selector
    uint32_t edi, esi, ebp, unused, ebx, edx, ecx, eax; // Pushed by pusha.
    uint32_t int_no, err_code; // Interrupt number and error code (if applicable)
    uint32_t eip, cs, eflags, useresp, ss; // Pushed by the processor automatically.
} Registers;

typedef void (*IDT_InterruptHandler)(Registers*);

extern char idt_handlers[IDT_ENTRY_COUNT][IDT_ENTRY_HANDLER_SIZE];
extern void(*idt_handler_calls[IDT_ENTRY_COUNT] ) (Registers*);

void IDT_Initialize();
void IDT_SetEntry(uint8_t index, uint32_t base, uint16_t selector, uint8_t flags);
void IDT_MainHandler(Registers *regs);
void IDT_RegisterHandler(uint8_t intNum, void (*handler)(Registers*));

#endif /* end of include guard: _IDT_H_ */
