#include "memorymanager.h"
#include "priv_memorymanager.h"

#include "elf.h"
#include "globals.h"

#include "pagetable.h"

#include "utils/common.h"


void MemMan_Initialize()
{
        //Determine which parts of memory are already in use by the kernel by reading the symbol table
        memory_size = 0;

        multiboot_memory_map_t* mmap = global_memory_map;
        while(mmap < global_memory_map + global_memory_map_size) {

                //Determine memory size
                if(mmap->type == MULTIBOOT_MEMORY_AVAILABLE) memory_size += (uint64_t)mmap->len;

                mmap = (multiboot_memory_map_t*) ( (unsigned int)mmap + mmap->size + sizeof(unsigned int) );
        }
        //Determine the total number of pages
        freePageCount = totalPageCount = page_count = memory_size / (uint64_t)PAGE_SIZE;
        lastNonFullPage = 0;
        lastFourthEmptyPage = 0;
        lastFourthFullPage = 0;
        lastHalfEmptyPage = 0;
        lastEmptyPage = 0;
        /*Allocate multiple page tables, of blocks of

           4KB, 8KB, 16KB, 32KB, 64KB, 128KB, 256KB, 512KB, 1MB

           How this works is each page table is divided into 1MB blocks
           Each 1MB block is divided into 8x128KB blocks
           Each 128KB block is divided into 32x4KB pages
         */

        KB4_Blocks_Count = memory_size/((uint64_t)KB(4) * 32);
        KB4_Blocks_Bitmap = Bootstrap_malloc(KB4_Blocks_Count * sizeof(uint32_t));

        KB4_Blocks_FreeBitCount_EntryNum = KB4_Blocks_Count / KB4_DIVISOR;
        KB4_Blocks_FreeBitCount = Bootstrap_malloc(KB4_Blocks_FreeBitCount_EntryNum * sizeof(uint32_t));

        if(KB4_Blocks_Bitmap == NULL) asm volatile ("int $0x9");

        memset(KB4_Blocks_Bitmap, 0, KB4_Blocks_Count * sizeof(uint32_t));

        for(int i = 0; i < KB4_Blocks_Count; i++)
        {
                SET_FREE_BITCOUNT(i, 32);
        }

        multiboot_elf_section_header_table_t *tmpHDR = &global_multiboot_info->u.elf_sec;

        Elf32_Shdr *hdr = (Elf32_Shdr*)tmpHDR->addr;
        for(int i = 0; i < tmpHDR->num; i++)
        {
                //Mark the corresponding pages as in use
                if(hdr->sh_size != 0)
                {
                        MemMan_MarkUsed(hdr->sh_addr, hdr->sh_size);
                }

                hdr++;
        }
}

uint64_t MemMan_CalculateBitmapIndex(uint64_t addr, size_t blockSize)
{
        addr /= blockSize;
        return addr;
}

void MemMan_MarkKB4Used(uint64_t addr, uint64_t size)
{
        uint64_t n_addr = KB(4) * (addr/KB(4));  //Make addr page aligned
        size += (addr - n_addr);

        uint64_t base_page = n_addr/(uint64_t)KB(4);
        uint64_t page_count = size / (uint64_t)KB(4);

        for(uint64_t i = base_page; i < page_count; i++)
        {
                KB4_Blocks_Bitmap[i/32] = SET_BIT(KB4_Blocks_Bitmap[i/32], (i % 32));

                DEC_FREE_BITCOUNT(i/32);
                freePageCount--;
                n_addr += KB(4);
        }
}

void MemMan_MarkUsed(uint64_t addr, uint64_t size)
{
        MemMan_MarkKB4Used(addr, size);
}

void* MemMan_Alloc(uint64_t size)
{
        if(size == 0) return NULL;

        uint64_t orig_size = size;
        uint64_t pageCount = size/KB(4);
        if(pageCount * KB(4) < size) pageCount++;

        int b = 0;

        uint64_t i = lastNonFullPage;

        if(pageCount <= GET_FREE_BITCOUNT(lastEmptyPage)) i = lastEmptyPage;
        if(pageCount <= GET_FREE_BITCOUNT(lastFourthFullPage)) i = lastFourthFullPage;
        if(pageCount <= GET_FREE_BITCOUNT(lastHalfEmptyPage)) i = lastHalfEmptyPage;
        if(pageCount <= GET_FREE_BITCOUNT(lastFourthEmptyPage)) i = lastFourthEmptyPage;

        //TODO For further optimization, determine fragmentation based on difference from nearest power of 2 from GET_FREE_BITCOUNT(i) + 1, if multiple choices are likely to work, pick the one which has the lowest fragmentation

        //If only one page is necessary, this one's a guaranteed solution
        if(pageCount == 1) i = lastNonFullPage;

        for(; i < KB4_Blocks_Count; i++)
        {
                if(pageCount == 1)
                {
                        if(KB4_Blocks_Bitmap[i] < 0xFFFFFFFF)
                        {
                                for(; b < 32; b++)
                                {
                                        if(!CHECK_BIT(KB4_Blocks_Bitmap[i], b))
                                        {
                                                break;
                                        }
                                }

                                KB4_Blocks_Bitmap[i] = SET_BIT(KB4_Blocks_Bitmap[i], b);
                                break;
                        }
                }
                else if(pageCount <= 32) {
                        if(KB4_Blocks_Bitmap[i] == 0)
                        {
                                //Definitely enough space
                                for(b = 0; b < pageCount; b++) {
                                        KB4_Blocks_Bitmap[i] = SET_BIT(KB4_Blocks_Bitmap[i], b);
                                }
                                break;
                        }else if(GET_FREE_BITCOUNT(i) >= pageCount) {
                                //asm ("bsf %1, %0" : "=r" (numFreePages) : "r" (KB4_Blocks_Bitmap[i]));
                                uint32_t numFreePages = 0;
                                int z = 0;
                                for(; z < 32; z++)
                                {
                                        if(!CHECK_BIT(KB4_Blocks_Bitmap[i], z)) numFreePages++;
                                        else if(numFreePages < pageCount) {
                                                numFreePages = 0;
                                        }else{
                                                break;
                                        }
                                }
                                b = z - numFreePages;

                                if(numFreePages >= pageCount)
                                {
                                        for(z = 0; z < pageCount; z++) {
                                                KB4_Blocks_Bitmap[i] = SET_BIT(KB4_Blocks_Bitmap[i], b);
                                                b++;
                                        }
                                        break;
                                }
                        }
                }
                else{
                        //TODO implement support for allocations larger than 32 pages
                        asm ("int $0x15");
                }
        }

        if(i >= KB4_Blocks_Count) {
                asm ("int $0x1");
                return NULL;
        }

        SET_FREE_BITCOUNT(i, GET_FREE_BITCOUNT(i) - pageCount);
        freePageCount -= pageCount;

        void *f_addr = (((i * 32) + b) * KB(4));
        if(pageCount > 1) f_addr = (uint64_t)f_addr - orig_size;

        //If the currently allocated page is full, find the next non-full page
        //if(KB4_Blocks_Bitmap[lastNonFullPage] == 0xFFFFFFFF) SET_FREE_BITCOUNT(lastNonFullPage, 0);
        while(GET_FREE_BITCOUNT(lastNonFullPage) == 0)
        {
                lastNonFullPage++ % KB4_Blocks_Count;
        }

        if(GET_FREE_BITCOUNT(lastEmptyPage) < 32) {

                while(GET_FREE_BITCOUNT(lastEmptyPage) < 32) lastEmptyPage++ % KB4_Blocks_Count;
        }

        if(GET_FREE_BITCOUNT(lastFourthFullPage) < 24 | (lastFourthFullPage == lastEmptyPage) | (lastFourthFullPage == lastNonFullPage))
        {
                while(GET_FREE_BITCOUNT(lastFourthFullPage) < 24 | (lastFourthFullPage == lastEmptyPage) | (lastFourthFullPage == lastNonFullPage)) lastFourthFullPage++ % KB4_Blocks_Count;
        }

        if(GET_FREE_BITCOUNT(lastHalfEmptyPage) < 16 | (lastHalfEmptyPage == lastEmptyPage) | (lastHalfEmptyPage == lastNonFullPage) | (lastHalfEmptyPage == lastFourthFullPage))
        {
                while(GET_FREE_BITCOUNT(lastHalfEmptyPage) < 16 | (lastHalfEmptyPage == lastEmptyPage) | (lastHalfEmptyPage == lastNonFullPage) | (lastHalfEmptyPage == lastFourthFullPage)) lastHalfEmptyPage++ % KB4_Blocks_Count;
        }

        if(GET_FREE_BITCOUNT(lastFourthEmptyPage) < 8 | (lastFourthEmptyPage == lastHalfEmptyPage) | (lastFourthEmptyPage == lastNonFullPage))
        {
                while(GET_FREE_BITCOUNT(lastFourthEmptyPage) < 8 | (lastFourthEmptyPage == lastHalfEmptyPage) | (lastFourthEmptyPage == lastNonFullPage)) lastFourthEmptyPage++ % KB4_Blocks_Count;
        }

        return f_addr;
}

void MemMan_Free(void *ptr, uint64_t size)
{
        if(size == 0) return;
        uint64_t addr = (uint64_t)ptr;
        uint64_t n_addr = KB(4) * (addr/KB(4));  //Make addr page aligned
        size += (addr - n_addr);

        uint64_t base_page = n_addr/(uint64_t)KB(4);
        uint64_t page_count = size / (uint64_t)KB(4);

        for(uint64_t i = base_page; i < base_page + page_count; i++)
        {
                KB4_Blocks_Bitmap[i/32] = CLEAR_BIT(KB4_Blocks_Bitmap[i/32], (i % 32));

                INC_FREE_BITCOUNT(i/32);
                freePageCount++;

                uint64_t i0 = i / 32;
                int bitCount = GET_FREE_BITCOUNT(i0);

                if(lastNonFullPage > i0) lastNonFullPage = i0;
                if(lastFourthEmptyPage > i0 && bitCount >= 8) lastHalfEmptyPage = i0;
                if(lastHalfEmptyPage > i0 && bitCount >= 16) lastHalfEmptyPage = i0;
                if(lastFourthFullPage > i0 && bitCount >= 24) lastHalfEmptyPage = i0;
                if(lastEmptyPage > i0 && bitCount == 32) lastHalfEmptyPage = i0;

        }
}
