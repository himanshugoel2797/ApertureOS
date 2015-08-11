#include "interruptmanager.h"

#include "utils/common.h"
#include "Graphics/graphics.h"
#include "drivers/drivers.h"

void (*interruptHandlers[IDT_ENTRY_COUNT][INTERRUPT_MANAGER_PRIORITY_COUNT]) (Registers*);

char *interruptMessages[32]=
{
        "#DE : Divide Error",
        "#DB : Debug",
        "#NMI : Non Maskable Interrupt",
        "#BP : Breakpoint",
        "#OF : Overflow",
        "#BR : Bound Range Exceeded",
        "#UD : Invalid Opcode",
        "#NM : Device Not Available",
        "#DF : Double Fault",
        "Coprocessor Segment Overrun (OS Boot Critical Exception)",
        "#TS : Invalid TSS",
        "#NP : Segment Not Present",
        "#SS : Stack Segment Fault",
        "#GP : General Protection Fault",
        "#PF : Page Fault",
        "Reserved (0xF)",
        "#MF : x87 FPU Exception (Should be disabled)",
        "#AC : Alignment Check",
        "#MC : Machine Check",
        "#XM/#XF : SIMD Floating Point Exception",
        "#VE : Virtualization Exception",
        "Reserved (0x15) : TODO Code Triggered! D:",
        "Reserved (0x16)",
        "Reserved (0x17)",
        "Reserved (0x18)",
        "Reserved (0x19)",
        "Reserved (0x1A)",
        "Reserved (0x1B)",
        "Reserved (0x1C)",
        "Reserved (0x1D)",
        "#SX : Security Exception",
        "Reserved (0x1F)"
};

void InterruptManager_InterruptHandler(Registers *regs);
void InterruptManager_Initialize()
{
        memset(interruptHandlers, 0, sizeof(interruptHandlers));
        for(uint8_t i = 0; i < 0xFF; i++)
        {
                IDT_RegisterHandler(i, InterruptManager_InterruptHandler);
        }
}

void InterruptManager_RegisterHandler(uint8_t int_no, uint8_t priority, void (*handler)(Registers*))
{
        interruptHandlers[int_no][priority] = handler;
}

void InterruptManager_InterruptHandler(Registers *regs)
{
        int handlerCalled = 0;
        for(int i = 0; i < INTERRUPT_MANAGER_PRIORITY_COUNT; i++)
        {
                if(interruptHandlers[regs->int_no][i] != NULL) {
                        interruptHandlers[regs->int_no][i](regs);
                        if(i < (INTERRUPT_MANAGER_PRIORITY_COUNT - 1)) handlerCalled = 1;
                }
        }

        //Show the exception message and stop if no handler was registered
        if(regs->int_no < 0x20 && handlerCalled == 0)
        {
                COM_WriteStr(interruptMessages[regs->int_no]);
                Graphics_WriteStr(interruptMessages[regs->int_no], 0, 0);
                Graphics_SwapBuffer();
        }else if(handlerCalled == 0) {
                COM_WriteStr("Interrupt %d\r\n", regs->int_no);
        }
        if(handlerCalled == 0)
                while(1) { asm volatile ("hlt"); }
}

uint8_t InterruptManager_GetFreePriority(uint8_t int_no)
{
        for(int i = INTERRUPT_MANAGER_LOWEST_PRIORITY; i > INTERRUPT_MANAGER_HIGHEST_PRIORITY; i--)
        {
                if(interruptHandlers[int_no][i] == NULL) return i;
        }
}
