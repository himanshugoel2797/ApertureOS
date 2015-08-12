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

uint32_t *apic_base_addr = 0;

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
                IDT_SetEntry(i, (uint32_t)idt_handlers[i], 0x08, 0x8E);
        }

        //Initialize the local APIC
        uint8_t apic_available = CPUID_FeatureIsAvailable(CPUID_EDX, CPUID_FEAT_EDX_APIC);
        if(!apic_available) return -1;

        uint64_t apic_base_msr = rdmsr(IA32_APIC_BASE);
        apic_base_addr = (uint32_t*)((uint32_t)apic_base_msr & 0xfffff000);
        apic_base_msr |= (1 << 11); //Enable the apic
        wrmsr(IA32_APIC_BASE, apic_base_msr);

        uint32_t tpr = APIC_Read(APIC_TPR);
        tpr &= ~(0xFF);
        APIC_Write(APIC_TPR, tpr);

        //Set the spurious vector
        uint32_t svr = APIC_Read(APIC_SVR);
        svr |= 0xFF;
        APIC_Write(APIC_SVR, svr);

        APIC_SetEnableMode(1);

        return 0;
}

void APIC_Write(uint32_t reg, uint32_t val)
{
        apic_base_addr[reg/4] = val;
}

uint32_t APIC_Read(uint32_t reg)
{
        return apic_base_addr[reg/4];
}

void APIC_SetEnableInterrupt(uint32_t interrupt, int enableMode)
{
        if(interrupt < 0x320 || interrupt > 0x360) return;
        uint32_t val = APIC_Read(interrupt);
        val = SET_VAL_BIT(val, 16, (~enableMode & 1));
        APIC_Write(interrupt, val);
}

void APIC_SetVector(uint32_t interrupt, uint8_t vector)
{
        if(interrupt < 0x320 || interrupt > 0x360) return;
        uint32_t val = APIC_Read(interrupt);
        val |= vector;
        APIC_Write(interrupt, val);
}

void APIC_SetEnableMode(uint8_t enabled)
{
        uint32_t svr = APIC_Read(APIC_SVR);
        svr = SET_VAL_BIT(svr, 8, (enabled & 1));
        APIC_Write(APIC_SVR, svr);
}

uint8_t APIC_GetID()
{
        return (uint8_t)APIC_Read(APIC_ID);
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
        uint32_t isr_msr = APIC_ISR_BASE + ((int_num / 32) * 0x10);
        uint32_t val = APIC_Read(isr_msr);
        if( CHECK_BIT(val, (int_num % 32)) ) {
                COM_WriteStr("Sending EOI for int %d\r\n", int_num);
                APIC_Write(APIC_EOI, 0xDEADBEEF);
        }
}

void APIC_MainHandler(Registers *regs)
{
        uint8_t int_num = regs->int_no;
        IDT_MainHandler(regs);
        APIC_SendEOI(int_num);
}
