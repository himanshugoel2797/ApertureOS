#include "priv_phys_mem_manager.h"
#include "elf.h"
#include "globals.h"
#include "utils/common.h"

bool memSearch_isFree(uint64_t addr)
{

        multiboot_elf_section_header_table_t *tmpHDR = &global_multiboot_info->u.elf_sec;

        Elf32_Shdr *hdr = (Elf32_Shdr *)tmpHDR->addr;
        multiboot_memory_map_t *mmap = global_memory_map;


        for (int i = 0; i < tmpHDR->num; i++) {

                if(hdr->sh_size > 0) {
                        uint64_t sh_addr = hdr->sh_addr;
                        // Mark the corresponding pages as in use
                        if (sh_addr <= addr && sh_addr + hdr->sh_size >= addr)
                        {
                                return FALSE;
                        }
                }

                hdr++;
        }

        while (mmap < global_memory_map + global_memory_map_size) {
                // Make sure this memory is not freeable
                if ((mmap->type != MULTIBOOT_MEMORY_AVAILABLE &&
                     mmap->type != MULTIBOOT_MEMORY_ACPI_RECLAIMABLE) &&
                    mmap->len != 0)
                {
                        if(mmap->addr <= addr && mmap->addr + mmap->len >= addr) {
                                return FALSE;
                        }
                }
                mmap = (multiboot_memory_map_t *)((unsigned int)mmap + mmap->size +
                                                  sizeof(unsigned int));
        }
        return TRUE;
}
