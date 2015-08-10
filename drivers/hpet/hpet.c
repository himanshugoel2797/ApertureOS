#include "hpet.h"
#include "priv_hpet.h"

#include "acpi_tables/acpi_tables.h"

uint8_t HPET_Initialize()
{
        hpet = ACPITables_FindTable(HPET_SIG);  //Find the HPET table
        frequency = 0;

        if(hpet == NULL) return -1; //No HPET, system should default to PIT

        //Get HPET period
        capabilities = *(uint64_t*)hpet->address.address;

        uint64_t numerator = 10;
        for(int i = 0; i < 15; i++) numerator *= 10;

        //Calculate the HPET frequency
        frequency = numerator/(capabilities >> 32);

        return (capabilities >> 8) & 0xF;
}

uint64_t HPET_GetGlobalCounter()
{
        if(hpet != NULL) {
                return *(uint64_t*)(hpet->address.address + MCV_OFFSET);
        }
        return 0;
}

void HPET_SetGlobalCounter(uint64_t val)
{
        if(hpet != NULL) {
                *(uint64_t*)(hpet->address.address + MCV_OFFSET) = val;
        }
}
