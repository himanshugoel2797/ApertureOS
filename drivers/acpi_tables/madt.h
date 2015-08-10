#ifndef _MADT_ACPI_TABLE_H_
#define _MADT_ACPI_TABLE_H_

#include <stddef.h>
#include <stdint.h>
#include "priv_acpi_tables.h"

typedef struct {
        ACPISDTHeader h;
        uint32_t local_controller_addr;
        uint32_t flags;
        uint8_t entries[1];
}MADT;

typedef struct {
        uint8_t type;
        uint8_t entry_size;
}MADT_EntryHeader;

typedef struct {
        MADT_EntryHeader h;
        uint8_t processor_id;
        uint8_t apic_id;
        uint32_t flags;
}MADT_EntryLAPIC;
#define MADT_LAPIC_ENTRY_TYPE 0

typedef struct {
        MADT_EntryHeader h;
        uint8_t io_apic_id;
        uint8_t reserved;
        uint32_t io_apic_base_addr;
        uint32_t global_sys_int_base;
}MADT_EntryIOAPIC;
#define MADT_IOAPIC_ENTRY_TYPE 1

typedef struct {
        MADT_EntryHeader h;
        uint8_t bus_src;
        uint8_t irq_src;
        uint32_t global_sys_int;
        uint16_t flags;
}MADT_EntryISAOVR;
#define MADT_ISAOVER_ENTRY_TYPE 2

#endif /* end of include guard: _MADT_ACPI_TABLE_H_ */
