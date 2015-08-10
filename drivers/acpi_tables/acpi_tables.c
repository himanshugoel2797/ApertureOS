#include "acpi_tables.h"
#include "priv_acpi_tables.h"

uint8_t ACPITables_Initialize()
{
        //Find the RSDP table
        uint8_t *rsdp_sig = (uint8_t*)BIOS_SEARCH_START;
        while( (uint32_t)rsdp_sig < BIOS_SEARCH_END)
        {
                if(rsdp_sig[0] == RSDP_EXPECTED_SIG[0])
                {
                        int pass = 1;

                        //Check the full signature
                        for(int i = 0; i < 8; i++)
                        {
                                if(rsdp_sig[i] != RSDP_EXPECTED_SIG[i]) pass = 0;
                        }

                        //Table found!
                        if(pass) {
                                rsdp = (RSDPDescriptor20*) rsdp_sig;
                                return rsdp->firstPart.Revision;
                        }
                }
                rsdp_sig += 16;         //Move ahead 16 bytes if the first
        }

        return -1;
}

uint8_t ACPITables_ValidateChecksum(ACPISDTHeader *header)
{
        unsigned char sum = 0;

        for (int i = 0; i < header->Length; i++)
        {
                sum += ((char *) header)[i];
        }

        return sum == 0;
}

void* ACPITables_FindTable(const char *table_name)
{
        RSDT *rsdt = (RSDT *) rsdp->firstPart.RsdtAddress;
        int entries = RSDT_GET_POINTER_COUNT(rsdt->h);

        for (int i = 0; i < entries; i++)
        {
                ACPISDTHeader *h = (ACPISDTHeader *) rsdt->PointerToOtherSDT[i];
                if (!strncmp(h->Signature, table_name, 4) && ACPITables_ValidateChecksum(h)) return (void *) h;
        }

        return NULL;
}
