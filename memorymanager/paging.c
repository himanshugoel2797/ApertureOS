#include "paging.h"
#include "priv_paging.h"
#include "memorymanager.h"

#include "cpuid.h"

#include "utils/common.h"

//Page directory pointer table
PDPT_Entry pdpt[4] __attribute__((aligned(0x20)));


void Paging_Initialize()
{
        //Make sure PAE is available
        if(!CPUID_FeatureIsAvailable(CPUID_EDX, CPUID_FEAT_EDX_PAE)) asm ("int $0x9");

        for(int i = 0; i < 4; i++)
        {
                pdpt[i].addr = SET_ADDR(MemMan_Alloc(KB(4)));
                pdpt[i].present = 0;
                pdpt[i].write_through = 1;
                pdpt[i].cache_disable = 0;
        }

        //Map everything to
}
