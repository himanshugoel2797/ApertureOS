#ifndef _PRIV_IO_APIC_DRIVER_H_
#define _PRIV_IO_APIC_DRIVER_H_

#include <stddef.h>
#include <stdint.h>

typedef struct {
        uint32_t baseAddr;
        uint32_t global_int_base;
        uint32_t ID;
}IOAPIC_Desc;

#endif /* end of include guard: _PRIV_IO_APIC_DRIVER_H_ */
