#ifndef _ACPI_TABLES_DRIVER_H_
#define _ACPI_TABLES_DRIVER_H_

#include "types.h"

#define MADT_SIG "APIC"
#define FADT_SIG "FACP"
#define HPET_SIG "HPET"
#define MCFG_SIG "MCFG"

uint8_t ACPITables_Initialize();
void* ACPITables_FindTable(const char *table_name, int index);

#endif /* end of include guard: _ACPI_TABLES_DRIVER_H_ */
