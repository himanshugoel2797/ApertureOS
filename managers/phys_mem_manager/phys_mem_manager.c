#include "phys_mem_manager.h"
#include "priv_phys_mem_manager.h"
#include "managers.h"

#include "globals.h"

SystemData *pmem_sys = NULL;
uint32_t pmem_Initialize();
void pmem_callback(uint32_t res);
uint8_t pmem_messageHandler(Message *msg);


void physMemMan_Setup(void) {
        pmem_sys = SysMan_RegisterSystem();
        strcpy(pmem_sys->sys_name, "physMemoryMan");

        pmem_sys->prerequisites[0] = 0;

        pmem_sys->init = pmem_Initialize;
        pmem_sys->init_cb = pmem_callback;
        pmem_sys->msg_cb = pmem_messageHandler;

        SysMan_StartSystem(pmem_sys->sys_id);
}

uint32_t pmem_Initialize() {

        memory_size = global_multiboot_info->mem_lower + global_multiboot_info->mem_upper;

        // Determine the total number of pages
        freePageCount = totalPageCount = memory_size / (uint64_t)PAGE_SIZE;

        //Populate the free stack
        freePageStackBase = bootstrap_malloc(PAGE_STACK_SIZE);
        freePageStackBase += PAGE_STACK_SIZE;
        freePageStack = freePageStackBase;

        for(uint64_t paddr = 0; paddr < memory_size; paddr+=PAGE_SIZE)
        {
                //Push vaddr onto the stack if the region is free acc to the mmap and elf table
                if(memSearch_isFree(paddr))
                {
                        freePageStack--;
                        *freePageStack = paddr;
                }
        }

        return 0;
}

uint64_t physMemMan_Alloc(void) {

        if(freePageStack != freePageStackBase)
        {
                return *freePageStack++;
        }
        return NULL;
}

void physMemMan_Free(uint64_t ptr) {
  //This shuold use some trick to ensure that the free stack can't be attacked
        freePageStack--;
        *freePageStack = ptr;
}

void pmem_callback(uint32_t res) {
}

uint8_t pmem_messageHandler(Message *msg) {
}
