#ifndef _PRIV_ACPI_TABLES_DRIVER_H_
#define _PRIV_ACPI_TABLES_DRIVER_H_

#include "types.h"

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
        uint64_t PointerToOtherSDT[1];
}XSDT;

typedef struct {
        ACPISDTHeader h;
        uint32_t PointerToOtherSDT[1];
}RSDT;

typedef struct
{
        uint8_t address_space_id; // 0 - system memory, 1 - system I/O
        uint8_t register_bit_width;
        uint8_t register_bit_offset;
        uint8_t reserved;
        uint64_t address;
} GenericAddressStructure;

#define XSDT_GET_POINTER_COUNT(h) ((h.Length - sizeof(h)) / 8)
#define RSDT_GET_POINTER_COUNT(h) ((h.Length - sizeof(h)) / 4)
#define BIOS_SEARCH_START 0x000E0000
#define BIOS_SEARCH_END   0x000FFFFF

uint8_t ACPITables_ValidateChecksum(ACPISDTHeader *header);
RSDPDescriptor20 *rsdp;

//TODO this should eventually go into the PCI base driver
typedef struct pci_vendor {
        uint16_t ven_id;
        const char *ven_name;
} pci_vendor_t;

typedef struct pci_device {
        uint16_t ven_id;
        uint16_t dev_id;
        const char *dev_name;
} pci_device_t;

typedef struct pci_baseclass {
        uint8_t baseclass;
        const char *name;
} pci_baseclass_t;

typedef struct pci_subclass {
        uint8_t baseclass;
        uint8_t subclass;
        const char *name;
} pci_subclass_t;


#endif /* end of include guard: _PRIV_ACPI_TABLES_DRIVER_H_ */
