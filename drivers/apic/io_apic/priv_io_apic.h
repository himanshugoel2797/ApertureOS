#ifndef _PRIV_IO_APIC_DRIVER_H_
#define _PRIV_IO_APIC_DRIVER_H_

#include "types.h"


uint32_t IOAPIC_Read(uint32_t* io_apic_baseAddr, uint32_t index);
void IOAPIC_Write(uint32_t* io_apic_baseAddr, uint32_t index, uint32_t val);


typedef struct {
        uint32_t baseAddr;
        uint32_t global_int_base;
        uint32_t ID;
        uint32_t entry_count;
}IOAPIC_Desc;

#endif /* end of include guard: _PRIV_IO_APIC_DRIVER_H_ */
