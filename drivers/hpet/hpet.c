#include "hpet.h"
#include "priv_hpet.h"

#include "acpi_tables/acpi_tables.h"

uint8_t HPET_Initialize()
{
        hpet = ACPITables_FindTable(HPET);  //Find the HPET table

        if(hpet == NULL) return -1; //No HPET, system should default to PIT
        
        //Get HPET period
        uint64_t capabilities = *(uint64_t*)hpet->address.address;

        uint64_t numerator = 10;
        for(int i = 0; i < 15; i++) numerator *= 10;

        //Calculate the HPET frequency
        frequency = numerator/(capabilities >> 32);

        return 0;
}
