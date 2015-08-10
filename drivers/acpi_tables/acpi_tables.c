#include "acpi_tables.h"
#include "priv_acpi_tables.h"

#include "utils/common.h"

uint8_t ACPITables_Initialize()
{
        rsdp = NULL;
        //Find the RSDP table
        uint8_t *rsdp_sig = (uint8_t*)BIOS_SEARCH_START;
        while( (uint32_t)rsdp_sig < BIOS_SEARCH_END)
        {
                if(rsdp_sig[0] == RSDP_EXPECTED_SIG[0])
                {
                        //Check the full signature
                        if(!strncmp(rsdp_sig, RSDP_EXPECTED_SIG, 8))
                        {
                                rsdp = (RSDPDescriptor20*) rsdp_sig;
                                uint32_t checksum = 0;

                                for(uint8_t *tmp = rsdp_sig; tmp < rsdp_sig + sizeof(RSDPDescriptor); tmp++) {
                                        checksum += *tmp;
                                }
                                if((checksum & 0xFF) == 0) return rsdp->firstPart.Revision;
                                else rsdp = NULL;
                        }
                }
                rsdp_sig += 16;         //Move ahead 16 bytes if the first
        }

        return -1;
}

uint8_t ACPITables_ValidateChecksum(ACPISDTHeader *header)
{
        uint8_t sum = 0;

        for (int i = 0; i < header->Length; i++)
        {
                sum += ((char *)header)[i];
        }

        return sum == 0;
}

void* ACPITables_FindTable(const char *table_name)
{
        if(rsdp == NULL) return NULL;

        if(rsdp->firstPart.Revision == ACPI_VERSION_1) {
                RSDT *rsdt = (RSDT *) rsdp->firstPart.RsdtAddress;
                if(!ACPITables_ValidateChecksum(rsdt)) return -1;

                int entries = RSDT_GET_POINTER_COUNT((rsdt->h));

                for (int i = 0; i < entries; i++)
                {
                        ACPISDTHeader *h = (ACPISDTHeader *) rsdt->PointerToOtherSDT[i];
                        if (!strncmp(h->Signature, table_name, 4) && ACPITables_ValidateChecksum(h)) return (void *) h;
                }
        }else{
                XSDT *xsdt = (XSDT*)rsdp->XsdtAddress;
                if(!ACPITables_ValidateChecksum(xsdt)) return -1;

                int entries = XSDT_GET_POINTER_COUNT((xsdt->h));

                for (int i = 0; i < entries; i++)
                {
                        ACPISDTHeader *h = (ACPISDTHeader *) xsdt->PointerToOtherSDT[i];
                        if (!strncmp(h->Signature, table_name, 4) && ACPITables_ValidateChecksum(h)) return (void *) h;
                }
        }

        return NULL;
}
