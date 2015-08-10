#ifndef _PRIV_HPET_DRIVER_H_
#define _PRIV_HPET_DRIVER_H_

#include <stddef.h>
#include <stdint.h>

#include "acpi_tables/priv_acpi_tables.h"

typedef struct
{
        ACPISDTHeader h;
        uint8_t hardware_rev_id;
        uint8_t comparator_count : 5;
        uint8_t counter_size : 1;
        uint8_t reserved : 1;
        uint8_t legacy_replacement : 1;
        pci_vendor_t pci_vendor_id;
        address_structure address;
        uint8_t hpet_number;
        uint16_t minimum_tick;
        uint8_t page_protection;
}HPET __attribute__((packed));

HPET *hpet = NULL;
uint64_t frequency = 0;

#endif /* end of include guard: _PRIV_HPET_DRIVER_H_ */
