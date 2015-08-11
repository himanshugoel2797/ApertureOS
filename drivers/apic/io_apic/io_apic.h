#ifndef _IO_APIC_DRIVER_H_
#define _IO_APIC_DRIVER_H_

#include <stddef.h>
#include <stdint.h>

#define MAX_IOAPIC_COUNT 128
#define IOAPIC_PIN_COUNT 24

uint8_t IOAPIC_Initialize(uint32_t baseAddr, uint32_t global_int_base, uint32_t id);
void IOAPIC_MapIRQ(uint8_t global_irq, uint8_t apic_vector, uint64_t apic_id, uint8_t trigger_mode, uint8_t polarity);

#endif /* end of include guard: _IO_APIC_DRIVER_H_ */
