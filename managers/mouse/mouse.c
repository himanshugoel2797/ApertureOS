#include "mouse.h"


static void
Mouse_IntHandler(Registers *regs);

void
Mouse_Initialize(void)
{
	Interrupts_RegisterHandler(IRQ(12), 0, Mouse_IntHandler);
	IOAPIC_SetEnableMode(IRQ(12), ENABLE);	
}

static void
Mouse_IntHandler(Registers *regs)
{
	while(inb(0x64) & 1)inb(0x60);
}