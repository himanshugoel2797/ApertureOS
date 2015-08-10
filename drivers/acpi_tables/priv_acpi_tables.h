#ifndef _PRIV_ACPI_TABLES_DRIVER_H_
#define _PRIV_ACPI_TABLES_DRIVER_H_

#include <stddef.h>
#include <stdint.h>

typedef struct {
        char Signature[8];
        uint8_t Checksum;
        char OEMID[6];
        uint8_t Revision;
        uint32_t RsdtAddress;
} RSDPDescriptor;

typedef struct {
        RSDPDescriptor firstPart;

        uint32_t Length;
        uint64_t XsdtAddress;
        uint8_t ExtendedChecksum;
        uint8_t reserved[3];
} RSDPDescriptor20;

#define RSDP_EXPECTED_SIG "RSD PTR "
#define ACPI_VERSION_1 0
#define ACPI_VERSION_2 1

typedef struct {
        char Signature[4];
        uint32_t Length;
        uint8_t Revision;
        uint8_t Checksum;
        char OEMID[6];
        char OEMTableID[8];
        uint32_t OEMRevision;
        uint32_t CreatorID;
        uint32_t CreatorRevision;
} ACPISDTHeader;

typedef struct {
        ACPISDTHeader h;
        uint64_t *PointerToOtherSDT;
}XSDT;

typedef struct {
        ACPISDTHeader h;
        uint32_t *PointerToOtherSDT;
}RSDT;

#define XSDT_GET_POINTER_COUNT(h) ((h.Length - sizeof(h)) / 8)
#define RSDT_GET_POINTER_COUNT(h) ((h.Length - sizeof(h)) / 4)
#define BIOS_SEARCH_START 0x000E0000
#define BIOS_SEARCH_END   0x000FFFFF

RSDPDescriptor20 *rsdp;

uint8_t ACPITables_ValidateChecksum(ACPISDTHeader *header);
void* ACPITables_FindTable(const char *table_name);

#endif /* end of include guard: _PRIV_ACPI_TABLES_DRIVER_H_ */
