#ifndef _MCFG_ACPI_H_
#define _MCFG_ACPI_H_

#include "types.h"
#include "acpi_tables.h"
#include "priv_acpi_tables.h"

typedef struct
{
    uint64_t baseAddr;
    uint16_t group_segment_number;
    uint8_t start_bus_number;
    uint8_t end_bus_number;
    uint32_t res0;
} MCFG_Entry;

typedef struct
{
    ACPISDTHeader h;
    uint64_t res0;
    MCFG_Entry entries[1];
} MCFG;

#endif /* end of include guard: _MCFG_ACPI_H_ */
