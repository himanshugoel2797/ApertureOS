#ifndef _ACPI_TABLES_DRIVER_H_
#define _ACPI_TABLES_DRIVER_H_

#include <stddef.h>
#include <stdint.h>

#define MADT_SIG "APIC"
#define FADT_SIG "FACP"
#define HPET_SIG "HPET"

uint8_t ACPITables_Initialize();
void* ACPITables_FindTable(const char *table_name);

#endif /* end of include guard: _ACPI_TABLES_DRIVER_H_ */
