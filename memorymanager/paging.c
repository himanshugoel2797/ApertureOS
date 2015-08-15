#include "paging.h"
#include "priv_paging.h"

#include "globals.h"
#include "elf.h"

#include "cpuid.h"

#include "utils/common.h"


void Paging_Initialize()
{
        //Stop if the necessary paging features aren't available
        if(!CPUID_FeatureIsAvailable(CPUID_EDX, CPUID_FEAT_EDX_PSE) | !CPUID_FeatureIsAvailable(CPUID_EDX, CPUID_FEAT_EDX_PSE36)) asm ("int $0x9");

        pd_nopse = MemMan_Alloc(KB(4));
        pd_pse = (PD_Entry_PSE*)pd_nopse;

        for(int i = 0; i < 1024; i++)
        {
                PD_NO_PSE_Entry_INIT(pd_nopse[i]);
        }

        //Map the last page to the page directory itself
        SET_ADDR(pd_nopse[1023], (uint32_t)pd_nopse);  //Set the address to the pre allocated page table
        pd_nopse[1023].present = 1;


        //Now identity map the kernel using 5 4MB pages
        uint32_t k_addr = 0;
        for(int i = 0; i < 1024; i++)
        {
                pd_pse[i].present = 1;
                pd_pse[i].large_page = 1;
                pd_pse[i].low_addr = (k_addr >> 22);
                pd_pse[i].res2 = 0;
                pd_pse[i].high_addr = 0;
                pd_pse[i].pat = 0;
                k_addr += MB(4);
        }
        //return;

        //Activate paging
        asm volatile (
                "mov %%eax, %%cr3\n\t"
                "mov %%cr4, %%eax\n\t"
                "or $0x10, %%eax\n\t"
                "mov %%eax, %%cr4\n\t"
                "mov %%cr0, %%eax\n\t"
                "or $0x80000000, %%eax\n\t"
                "mov %%eax, %%cr0"
                :: "a" (pd_pse)
                );
        //while(1) {}
}

//TODO pass pdpt * as an argument to an internal copy of this function
void Paging_MapPage(uint64_t physAddress, uint32_t virtualAddress, int userAccessible, int pse)
{
        if(pse)
        {
                MB4_t addr;
                addr.directory_index = virtualAddress/MB(4);

                pd_pse[addr.directory_index].present = 1;
                pd_pse[addr.directory_index].large_page = 1;
                pd_pse[addr.directory_index].low_addr = (physAddress >> 22);
                pd_pse[addr.directory_index].res2 = 0;
                pd_pse[addr.directory_index].high_addr = (physAddress >> 32);
                pd_pse[addr.directory_index].pat = 0;
                pd_pse[addr.directory_index].user_supervisor = userAccessible;
        }else
        {
                KB4_t addr;
                addr.directory_index = virtualAddress/MB(4);
                addr.table_index = (virtualAddress % MB(4))/KB(4);
                addr.offset = (virtualAddress % KB(4));

                if(!pd_nopse[addr.directory_index].present) {
                        //Allocate a new page, flush the TLB, update the page

                        pd_nopse[addr.directory_index].present = 1;
                        pd_nopse[addr.directory_index].addr = ((uint32_t)MemMan_Alloc(KB(4)))/KB(4);
                        Paging_FlushAll();
                }

                //Determine the new address of the page
                KB4_t pageAddr;
                pageAddr.directory_index = 1023;
                pageAddr.table_index = addr.directory_index;
                pageAddr.offset = 0;

                PT_Entry *entry = *(PT_Entry**)&pageAddr;
                entry[addr.table_index].present = 1;
                entry[addr.table_index].addr = ((uint32_t)MemMan_Alloc(KB(4)))/KB(4);
                entry[addr.table_index].user_supervisor = userAccessible;


        }
}

void Paging_Flush(void *addr) {
        asm volatile ("invlpg (%0)" :: "r" (addr));
}

void Paging_FlushAll()
{
        asm volatile (
                "push %eax\n\t"
                "mov %cr3, %eax\n\t"
                "mov %eax, %cr3\n\t"
                "pop %eax\n\t"
                );
}
