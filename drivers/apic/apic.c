#include "apic.h"
#include "priv_apic.h"
#include "pic/pic.h"

#include "idt.h"
#include "cpuid.h"
#include "serial/COM.h"

#include "utils/common.h"
#include "utils/native.h"

#include "interruptmanager.h"

void APIC_EOI_HANDLER(Registers *Regs)
{
        APIC_SendEOI(Regs->int_no);
}

uint8_t APIC_LocalInitialize()
{
        PIC_Initialize(); //Initialize the PIC
        PIC_MaskAll();    //Temporarily disable all interrupts from the PIC

        for(int i = 0; i < 32; i++) {
                InterruptManager_RegisterHandler(i, INTERRUPT_MANAGER_PRIORITY_COUNT - 1, APIC_EOI_HANDLER);  //The very last handler is always the EOI
        }

        //Register the APIC interrupt handlers
        for(int i = 32; i < IDT_ENTRY_COUNT; i++) {
                APIC_FillHWInterruptHandler(idt_handlers[i], i, i - 32);
                IDT_SetEntry(i, idt_handlers[i], 0x08, 0x8E);
        }

        //Initialize the local APIC
        uint8_t apic_available = CPUID_FeatureIsAvailable(CPUID_EDX, CPUID_FEAT_EDX_APIC);
        uint8_t x2apic_available = CPUID_FeatureIsAvailable(CPUID_ECX, CPUID_FEAT_ECX_x2APIC);

        if(!apic_available) return -1;
        if(!x2apic_available) return -2;

        uint64_t apic_base_msr = rdmsr(IA32_APIC_BASE);
        apic_base_msr |= (3 << 10); //Enable both apic and x2apic mode
        wrmsr(IA32_APIC_BASE, apic_base_msr);

        //Set the spurious vector
        uint64_t svr = rdmsr(APIC_SVR);
        svr |= 0xFF;
        wrmsr(APIC_SVR, svr);

        //Software enable the APIC by setting the msr
        APIC_SetEnableMode(1);

        return 0;
}

void APIC_SetEnableMode(uint8_t enabled)
{
        uint64_t svr = rdmsr(APIC_SVR);
        svr = SET_VAL_BIT(svr, 8, enabled & 1);
        wrmsr(APIC_SVR, svr);
}

uint8_t APIC_GetID()
{
        return (uint8_t)rdmsr(APIC_ID);
}

void APIC_FillHWInterruptHandler(char *idt_handler, uint8_t intNum, uint8_t irqNum)
{
        int index = 0;

        //Push dummy error code if the interrupt doesn't do so
        idt_handler[index++] = 0x68;         //Push
        idt_handler[index++] = irqNum;
        idt_handler[index++] = 0;
        idt_handler[index++] = 0;
        idt_handler[index++] = 0;

        idt_handler[index++] = 0x68; //Push
        idt_handler[index++] = intNum; //Push the interrupt number to stack
        idt_handler[index++] = 0;
        idt_handler[index++] = 0;
        idt_handler[index++] = 0;

        //push jump address and ret
        idt_handler[index++] = 0x68;
        idt_handler[index++] = (int)APIC_DefaultHandler & 0x000000FF;  //Calculate the offset
        idt_handler[index++] = ((int)APIC_DefaultHandler >> 8)& 0x000000FF;  //Calculate the offset
        idt_handler[index++] = ((int)APIC_DefaultHandler >> 16)& 0x000000FF;  //Calculate the offset
        idt_handler[index++] = ((int)APIC_DefaultHandler >> 24)& 0x000000FF;  //Calculate the offset

        idt_handler[index++] = 0xC3;
}

void APIC_MainHandler(Registers *regs);
__attribute__((naked, noreturn))
void APIC_DefaultHandler()
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
                "call APIC_MainHandler"
                );
        asm (
                "pop %ebx\n\t"
                "pop %ebx\n\t"
                "mov %bx, %ds\n\t"
                "mov %bx, %es\n\t"
                "mov %bx, %fs\n\t"
                "mov %bx, %gs\n\t"
                "popa\n\t"
                "pop %eax\n\t"
                "add $4, %esp\n\t"
                "iret\n\t"
                );
}

void APIC_SendEOI(uint8_t int_num)
{
//Test if this is in service and send EOI
        uint32_t isr_msr = APIC_ISR_BASE + (int_num / 32);
        uint64_t val = rdmsr(isr_msr);
        if( CHECK_BIT(val, (int_num % 32)) ) {
                wrmsr(APIC_EOI, 0xDEADBEEF);
        }
}

void APIC_MainHandler(Registers *regs)
{
        uint8_t int_num = regs->int_no;
        IDT_MainHandler(regs);
        APIC_SendEOI(int_num);
}
