#ifndef _ACPI_TABLES_DRIVER_H_
#define _ACPI_TABLES_DRIVER_H_

#include <stddef.h>
#include <stdint.h>

#define MADT "APIC"
#define FADT "FACP"
#define HPET "HPET"

uint8_t ACPITables_Initialize();
void* ACPITables_FindTable(const char *table_name);

#endif /* end of include guard: _ACPI_TABLES_DRIVER_H_ */
