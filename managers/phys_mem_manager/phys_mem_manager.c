#include "phys_mem_manager.h"
#include "priv_phys_mem_manager.h"
#include "managers.h"

#include "elf.h"
#include "globals.h"
#include "utils/common.h"

SystemData *pmem_sys = NULL;
uint32_t pmem_Initialize();
void pmem_callback(uint32_t res);
uint8_t pmem_messageHandler(Message *msg);

uint64_t memory_size = 0;
uint16_t curIndex = 0;

void physMemMan_Setup() {
        pmem_sys = SysMan_RegisterSystem();
        strcpy(pmem_sys->sys_name, "physMemoryMan");

        pmem_sys->prerequisites[0] = 0;

        pmem_sys->init = pmem_Initialize;
        pmem_sys->init_cb = pmem_callback;
        pmem_sys->msg_cb = pmem_messageHandler;

        SysMan_StartSystem(pmem_sys->sys_id);
}

uint32_t pmem_Initialize() {

        // Determine which parts of memory are already in use by the kernel by reading
        // the symbol table
        memory_size = KB((global_multiboot_info->mem_lower + global_multiboot_info->mem_upper));



        // Determine the total number of pages
        freePageCount = totalPageCount = page_count = memory_size / (uint64_t)PAGE_SIZE;
        lastNonFullPage = 0;

        KB4_Blocks_Count = memory_size / ((uint64_t)KB(4) * 32);
        KB4_Blocks_Bitmap = bootstrap_malloc(KB4_Blocks_Count * sizeof(uint32_t));

        KB4_Blocks_FreeBitCount_EntryNum = KB4_Blocks_Count / KB4_DIVISOR;
        KB4_Blocks_FreeBitCount = bootstrap_malloc(KB4_Blocks_FreeBitCount_EntryNum * sizeof(uint32_t));

        memset(KB4_Blocks_Bitmap, 0, KB4_Blocks_Count * sizeof(uint32_t));
        COM_WriteStr("BlockSCuont %d", KB4_Blocks_FreeBitCount);

        for (int i = 0; i < KB4_Blocks_Count; i++) {
                SET_FREE_BITCOUNT(i, 32);
        }

        multiboot_elf_section_header_table_t *tmpHDR = &global_multiboot_info->u.elf_sec;

        Elf32_Shdr *hdr = (Elf32_Shdr*)tmpHDR->addr;
        for (int i = 0; i < tmpHDR->num; i++) {

                // Mark the corresponding pages as in use
                if (hdr->sh_size != 0) {
                        MemMan_MarkUsed(hdr->sh_addr, hdr->sh_size);
                }

                hdr++;
        }


        multiboot_memory_map_t *mmap = global_memory_map;
        while (mmap < global_memory_map + global_memory_map_size) {
                // Make sure this memory is not freeable
                if ((mmap->type != MULTIBOOT_MEMORY_AVAILABLE &&
                     mmap->type != MULTIBOOT_MEMORY_ACPI_RECLAIMABLE) | mmap->len != 0)
                        MemMan_MarkUsed(mmap->addr, mmap->len);
                mmap = (multiboot_memory_map_t *)((unsigned int)mmap + mmap->size +
                                                  sizeof(unsigned int));
        }

        return 0;
}

uint64_t MemMan_CalculateBitmapIndex(uint64_t addr, size_t blockSize) {
        addr /= blockSize;
        return addr;
}

void MemMan_MarkKB4Used(uint64_t addr, uint64_t size) {
        uint64_t n_addr = KB(4) * (addr / KB(4)); // Make addr page aligned
        size += (addr - n_addr);

        uint64_t base_page = n_addr / (uint64_t)KB(4);
        uint64_t page_count = size / (uint64_t)KB(4);

                COM_WriteStr("TEST!!! %d", page_count);
        for (uint64_t i = base_page; i < base_page + page_count; i++) {
                //KB4_Blocks_Bitmap[i / 32] = SET_BIT(KB4_Blocks_Bitmap[i / 32], (i % 32));

                //DEC_FREE_BITCOUNT(i / 32);
                //freePageCount--;
                //n_addr += KB(4);
        }
}

void MemMan_MarkUsed(uint64_t addr, uint64_t size) {
        MemMan_MarkKB4Used(addr, size);
}

uint32_t find_first_zero(uint32_t bit_array)
{
    uint32_t pos = 0;
    if(bit_array == 0)return 0;

    __asm__("bsfl %1,%0\n\t"
        "jne 1f\n\t"
        "movl $32, %0\n"
        "1:"
        : "=r" (pos)
        : "r" (~(bit_array)));

    return (unsigned short) pos;
}

uint64_t physMemMan_Alloc() {

        int b = 0;
        uint64_t i = lastNonFullPage;

        for (; i < KB4_Blocks_Count; i++) {
                        if (KB4_Blocks_Bitmap[i] < 0xFFFFFFFF) {

                                b = find_first_zero(KB4_Blocks_Bitmap[i]);
                                KB4_Blocks_Bitmap[i] = SET_BIT(KB4_Blocks_Bitmap[i], b);
                                break;
                        }
        }

        if (i >= KB4_Blocks_Count) {
                return NULL;
        }

        DEC_FREE_BITCOUNT(i);
        freePageCount--;

        uint64_t f_addr = (((i * 32) + b) * KB(4));

        while (GET_FREE_BITCOUNT(lastNonFullPage) == 0) {
                lastNonFullPage = (lastNonFullPage + 1) % KB4_Blocks_Count;
        }

        return f_addr;
}

void physMemMan_Free(uint64_t ptr) {

        uint64_t addr = (uint64_t)ptr;
        uint64_t n_addr = KB(4) * (addr / KB(4)); // Make addr page aligned

        uint64_t base_page = n_addr / (uint64_t)KB(4);

        for (uint64_t i = base_page; i < base_page + 1; i++) {
                KB4_Blocks_Bitmap[i / 32] = CLEAR_BIT(KB4_Blocks_Bitmap[i / 32], (i % 32));

                INC_FREE_BITCOUNT(i / 32);
                freePageCount++;

                uint64_t i0 = i / 32;
                int bitCount = GET_FREE_BITCOUNT(i0);

                if (lastNonFullPage > i0)
                        lastNonFullPage = i0;
        }
}

void pmem_callback(uint32_t res) {
}

uint8_t pmem_messageHandler(Message *msg) {
}
