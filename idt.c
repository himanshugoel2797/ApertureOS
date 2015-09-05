#include "idt.h"


void IDT_DefaultHandler();
void IDT_FillSWInterruptHandler(char *idt_handler, uint8_t intNum, uint8_t pushToStack);

typedef struct
{
    uint16_t base_lo; // offset bits 0..15
    uint16_t selector; // a code segment selector in GDT or LDT
    uint8_t zero; // unused, set to 0
    uint8_t type_attr; // type and attributes, see below
    uint16_t base_hi; // offset bits 16..31
} __attribute__((packed)) IDTEntry;

//Describes the pointer to the IDT entry
typedef struct
{
    uint16_t limit; //Size
    uint32_t base; //Location
} __attribute__((packed)) IDTPtr;

IDTEntry idt_entries[IDT_ENTRY_COUNT];
IDTPtr idt_table;
char idt_handlers[IDT_ENTRY_COUNT][IDT_ENTRY_HANDLER_SIZE];
void(*idt_handler_calls[IDT_ENTRY_COUNT] ) (Registers*);

void IDT_Initialize()
{
    idt_table.limit = (sizeof(IDTEntry) * IDT_ENTRY_COUNT) - 1;
    idt_table.base = (uint32_t)&idt_entries;

    //ensure interrupts are disabled
    asm ("cli");
    asm ("lidt (%0)" :: "r" (&idt_table));         //Load the IDT

    for(int i = 0; i < IDT_ENTRY_COUNT; i++)
    {
        IDT_SetEntry(0, 0, 0, 0);
        idt_handler_calls[i] = NULL;
    }

    //Fill the IDT
    int pushesToStack = 1;
    for(int i = 0; i < IDT_ENTRY_COUNT; i++)
    {
        //Setup the hardware interrupts
        if(i == 8 || (i >= 10 && i <= 14)) pushesToStack = 0;
        IDT_FillSWInterruptHandler(idt_handlers[i], i, pushesToStack);  //If pushesToStack is non-zero, the value will be pushed to stack
        IDT_SetEntry(i, (uint32_t)idt_handlers[i], 0x08, 0x8E);
        pushesToStack = 1;
    }

    return;
}

void IDT_SetEntry(uint8_t index, uint32_t base, uint16_t selector, uint8_t flags)
{
    idt_entries[index].base_lo = base & 0x0000FFFF;
    idt_entries[index].base_hi = (base >> 16) & 0x0000FFFF;
    idt_entries[index].type_attr = flags;
    idt_entries[index].zero = 0;
    idt_entries[index].selector = selector;
}

void IDT_FillSWInterruptHandler(char *idt_handler, uint8_t intNum, uint8_t pushToStack)
{
    int index = 0;

    //Push dummy error code if the interrupt doesn't do so
    if(pushToStack)
    {
        idt_handler[index++] = 0x68; //Push
        idt_handler[index++] = pushToStack;
        idt_handler[index++] = 0;
        idt_handler[index++] = 0;
        idt_handler[index++] = 0;
    }

    idt_handler[index++] = 0x68; //Push
    idt_handler[index++] = intNum; //Push the interrupt number to stack
    idt_handler[index++] = 0;
    idt_handler[index++] = 0;
    idt_handler[index++] = 0;

    //push jump address and ret
    idt_handler[index++] = 0x68;
    idt_handler[index++] = (int)IDT_DefaultHandler & 0x000000FF;  //Calculate the offset
    idt_handler[index++] = ((int)IDT_DefaultHandler >> 8)& 0x000000FF;  //Calculate the offset
    idt_handler[index++] = ((int)IDT_DefaultHandler >> 16)& 0x000000FF;  //Calculate the offset
    idt_handler[index++] = ((int)IDT_DefaultHandler >> 24)& 0x000000FF;  //Calculate the offset

    idt_handler[index++] = 0xC3;
}

__attribute__((naked, noreturn))
void IDT_DefaultHandler()
{
    asm (
        "pusha\n\t"
        "mov %ds, %ax\n\t"
        "push %eax\n\t"
        "mov $0x10, %ax\n\t"
        "mov %ax, %ds\n\t"
        "mov %ax, %es\n\t"
        "mov %ax, %fs\n\t"
        "mov %ax, %gs\n\t"
        "push %esp\n\t"
    );

    asm (
        "call IDT_MainHandler"
    );

    asm (
        "pop %eax\n\t"
        "pop %eax\n\t"
        "mov %ax, %ds\n\t"
        "mov %ax, %es\n\t"
        "mov %ax, %fs\n\t"
        "mov %ax, %gs\n\t"
        "popa\n\t"
        "add $8, %esp\n\t"
        "iret\n\t"
    );
}

void IDT_MainHandler(Registers *regs)
{
    if(idt_handler_calls[regs->int_no] != NULL) idt_handler_calls[regs->int_no](regs);
}

void IDT_RegisterHandler(uint8_t intNum, void (*handler)(Registers*))
{
    idt_handler_calls[intNum] = handler;
}
