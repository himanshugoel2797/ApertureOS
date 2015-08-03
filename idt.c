#include <stddef.h>
#include <stdint.h>

#include "idt.h"

void IDT_SetEntry(uint8_t index, uint32_t base, uint16_t selector, uint8_t flags);

typedef struct {
        uint16_t base_lo; // offset bits 0..15
        uint16_t selector; // a code segment selector in GDT or LDT
        uint8_t zero; // unused, set to 0
        uint8_t type_attr; // type and attributes, see below
        uint16_t base_hi; // offset bits 16..31
}IDTEntry;

//Describes the pointer to the IDT entry
typedef struct {
        uint16_t limit; //Size
        uint32_t base; //Location
}__attribute__((packed)) IDTPtr;

IDTEntry idt_entries[IDT_ENTRY_COUNT];
IDTPtr idt_table;

void IDT_Initialize()
{
        idt_table.limit = (sizeof(IDTEntry) * IDT_ENTRY_COUNT) - 1;
        idt_table.base = &idt_entries;

        //ensure interrupts are disabled
        asm ("cli");

        //Fill the IDT


        asm ("lidt (%0)" :: "r" (&idt_table)); //Load the IDT
        asm ("sti"); //Enable interrupts

        return;
}

void IDT_SetEntry(uint8_t index, uint32_t base, uint16_t selector, uint8_t flags)
{
    idt_entries[index].base_lo = base & 0x0000FFFF;
    idt_entries[index].base_hi = (base >> 16);
    idt_entries[index].type_attr = flags;
    idt_entries[index].zero = 0;
    idt_entries[index].selector = selector;
}
