#ifndef _PRIV_APIC_DRIVER_H_
#define _PRIV_APIC_DRIVER_H_

#include <stddef.h>
#include <stdint.h>

#define IA32_APIC_BASE 0x01B

#define APIC_ID 0x802
#define APIC_EOI 0x80B
#define APIC_SVR 0x80F
#define APIC_ISR_BASE 0x810

void APIC_SendEOI(uint8_t int_num);
void APIC_FillHWInterruptHandler(char *idt_handler, uint8_t intNum, uint8_t irqNum);
void APIC_DefaultHandler();

#endif /* end of include guard: _PRIV_APIC_DRIVER_H_ */
