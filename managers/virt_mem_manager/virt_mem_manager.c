#include "virt_mem_manager.h"
#include "priv_virt_mem_manager.h"
#include "managers.h"
#include "globals.h"
#include "utils/common.h"
#include "utils/native.h"

SystemData *vmem_sys = NULL;
uint32_t virtMemMan_Initialize();
void virtMemMan_callback(uint32_t res);
uint8_t virtMemMan_messageHandler(Message *msg);

VirtMemMan_Instance curInstance_virt;

//Identity mapped storage for page directories
uint8_t page_dir_storage[PAGE_DIR_STORAGE_POOL_SIZE] __attribute__((aligned(KB(4))));

//Identity mapped storage for unique pdpt entries
uint64_t pdpt_storage[PDPT_STORAGE_SIZE_U64] __attribute__((aligned(0x20)));

PD_Entry_PSE *kernel_main_entry;

void virtMemMan_Setup()
{
        vmem_sys = SysMan_RegisterSystem();
        strcpy(vmem_sys->sys_name, "virtMemoryMan");
        vmem_sys->prerequisites[0] = 0;     //No prereqs
        vmem_sys->init = virtMemMan_Initialize;
        vmem_sys->init_cb = virtMemMan_callback;
        vmem_sys->msg_cb = virtMemMan_messageHandler;

        SysMan_StartSystem(vmem_sys->sys_id);
}

uint32_t virtMemMan_Initialize()
{
        //Update the PAT so that PAT4 is WC type allowing us to use the 4 types for pages
        uint64_t pat = 0;
        pat |= 0x6;                   //PAT0 WB
        pat |= ((uint64_t)0x4) << 8;  //PAT1 WT
        pat |= ((uint64_t)0x0) << 16; //PAT2 UC
        pat |= ((uint64_t)0x1) << 24;  //PAT3
        wrmsr(PAT_MSR, pat);

        memset(page_dir_storage, 0xFF, PAGE_DIR_STORAGE_POOL_SIZE);
        memset(pdpt_storage, 0xFF, PDPT_STORAGE_SIZE_U64 * sizeof(uint64_t));

        kernel_main_entry = virtMemMan_GetFreePageDirEntry();
        memset(kernel_main_entry, 0, KB(4));

        curInstance_virt = virtMemMan_CreateInstance();

        //TODO we might want to make this more secure by parsing info from hte elf table and making sections NX appropriately
        virtMemMan_Map(LOAD_ADDRESS, LOAD_ADDRESS, 0x10000000 - LOAD_ADDRESS, MEM_TYPE_WB, MEM_WRITE | MEM_READ | MEM_EXEC, MEM_KERNEL);

        virtMemMan_Map(0x10000000, 0xF0000000, 0x10000000, MEM_TYPE_UC, MEM_WRITE | MEM_READ, MEM_KERNEL);

        wrmsr(0xC0000080, rdmsr(0xC0000080) | (1<<11)); //Enable NXE

        asm volatile ("movl %cr4, %eax; orl $0x00000010, %eax; movl %eax, %cr4;");  //Enable PSE
        asm volatile ("movl %cr4, %eax; bts $5, %eax; movl %eax, %cr4"); // set bit5 in CR4 to enable PAE
        asm volatile ("movl %%eax, %%cr3" :: "a" (curInstance_virt)); // load PDPT into CR3
        asm volatile ("movl %cr0, %eax; orl $0x80000000, %eax; movl %eax, %cr0;");  //Enable Paging
        return 0;
}

void virtMemMan_callback(uint32_t res)
{

}

uint8_t virtMemMan_messageHandler(Message *msg)
{

}

VirtMemMan_Instance virtMemMan_SetCurrent(VirtMemMan_Instance *instance)
{
        VirtMemMan_Instance prev = curInstance_virt;
        curInstance_virt = instance;
        asm volatile ("movl %%eax, %%cr3" :: "a" (curInstance_virt)); // cause a TLB cache flush
        return prev;
}

VirtMemMan_Instance virtMemMan_GetCurrent()
{
        return curInstance_virt;
}

VirtMemMan_Instance virtMemMan_CreateInstance()
{
        //Identity map up to 0x40000000 assuming virtual memory is enabled
        VirtMemMan_Instance instance = virtMemMan_GetFreePDPTEntry();
        memset(instance, 0, sizeof(uint64_t) * 4);

        instance[0] = kernel_main_entry;
        instance[1] = virtMemMan_GetFreePageDirEntry();
        instance[2] = virtMemMan_GetFreePageDirEntry();
        instance[3] = virtMemMan_GetFreePageDirEntry();

        memset((void*)instance[1], 0, KB(4));
        memset((void*)instance[2], 0, KB(4));
        memset((void*)instance[3], 0, KB(4));

        return instance;  //The rest should be setup later
}

void virtMemMan_Fork(VirtMemMan_Instance dst, VirtMemMan_Instance src)
{
        memcpy(&dst[1], &src[1], KB(4));
        memcpy(&dst[2], &src[2], KB(4));
        memcpy(&dst[3], &src[3], KB(4));
}

void virtMemMan_ForkCurrent(VirtMemMan_Instance dst)
{
        virtMemMan_Fork(dst, curInstance_virt);
}

void* virtMemMan_FindEmptyAddress(size_t size, MEM_SECURITY_PERMS privLevel)
{
        if(size == 0) return NULL;

        uint32_t seg_cnt = size/MB(2);
        if(seg_cnt == 0 || ((seg_cnt * MB(2)) < size)) seg_cnt++;

        for(int i_pdpt = (privLevel == MEM_KERNEL) ? 0 : 1; i_pdpt < 4; i_pdpt++ )
        {
                if((curInstance_virt[i_pdpt] & 1) == 1) {

                        PD_Entry_PSE *pd_pse = GET_ADDR(&curInstance_virt[i_pdpt]);
                        for(int i_pd = 0; i_pd < 512; i_pd++)
                        {
                                if(pd_pse[i_pd].present == FALSE) {

                                        //TODO this ought to be able to search across pdpt boundaries

                                        int score = 0;
                                        for(int j = 0; j < seg_cnt && (i_pd + j) < 512; j++)
                                        {
                                                if(pd_pse[i_pd + j].present == FALSE) score++;
                                        }

                                        if(score >= seg_cnt) return (GB(1) * i_pdpt) + (MB(2) * i_pd);
                                }else if(pd_pse[i_pd].page_size == 0 && size < MB(2))
                                {
                                        uint64_t *tmp_pd_u64 = (uint64_t*)pd_pse;
                                        PT_Entry *pt = (tmp_pd_u64[i_pd] & 0xfffff000);
                                        seg_cnt = size/KB(4);
                                        if(seg_cnt == 0 || ((seg_cnt * KB(4)) < size)) seg_cnt++;

                                        for(int j = 0; j < 512; j++)
                                        {
                                                if(pt[j].present == FALSE) {
                                                        int score = 0;
                                                        for(int k = 0; k < seg_cnt && (j + k) < 512; k++)
                                                        {
                                                                if(pt[j + k].present == FALSE) score++;
                                                        }
                                                        if(score >= seg_cnt) return (i_pdpt * GB(1) + (i_pd * MB(2)) + (j * KB(4)));
                                                }
                                        }
                                }
                        }
                }else{
                        if(seg_cnt <= 512) return GB(1) * i_pdpt;
                }
        }

        return NULL;
}

uint32_t virtMemMan_Map(uint32_t v_address, uint64_t phys_address, size_t size, MEM_TYPES type, MEM_ACCESS_PERMS perms, MEM_SECURITY_PERMS privLevel)
{
        if(size == 0) return -1;

        uint32_t virtAddr = (uint32_t)v_address;
        uint64_t physAddr = (uint64_t)phys_address;

        //Check requested permissions to make sure they match up with the virtual address
        if(virtAddr < KMEM_END && privLevel != MEM_KERNEL) return -2;                 //Make sure permissions match
        if(virtAddr > KMEM_END && privLevel != MEM_USER) return -2;
        if(virtAddr + size > KMEM_END && virtAddr < KMEM_END) return -3;                 //Don't allow boundary crossing

        //Calculate the indices
        uint32_t pdpt_i = virtAddr/GB(1);
        uint32_t pd_i = (virtAddr - (pdpt_i * GB(1)))/MB(2);
        uint32_t pt_i = (virtAddr - (pdpt_i * GB(1)) - (pd_i * MB(2)))/KB(4);

        if(size == MB(2))
        {
                //Align the virtAddr to 2MB
                //virtAddr = (virtAddr/MB(2)) * MB(2);

                curInstance_virt[pdpt_i] |= 1;

                //Now update the current page directory
                PD_Entry_PSE *pd_pse = (PD_Entry_PSE*)GET_ADDR(&curInstance_virt[pdpt_i]);

                pd_pse[pd_i].addr = physAddr/MB(2);
                pd_pse[pd_i].present = 1;
                pd_pse[pd_i].user_supervisor = privLevel;
                pd_pse[pd_i].page_size = 1;
                pd_pse[pd_i].read_write = (perms & MEM_WRITE == MEM_WRITE);
                pd_pse[pd_i].nx = (perms & MEM_EXEC != MEM_EXEC);

                //Setup cache controls
                pd_pse[pd_i].global = 0;
                pd_pse[pd_i].pat = 0;
                pd_pse[pd_i].write_through = (uint32_t)type & 1;
                pd_pse[pd_i].cache_disable = ((uint32_t)type >> 1) & 1;

                //Flush the TLB
                asm volatile ("invlpg (%0)" :: "r" (virtAddr));

        }
        else if(size == KB(4))
        {
                //Align the virtAddr to 4KB
                //virtAddr = (virtAddr/KB(4)) * KB(4);

                curInstance_virt[pdpt_i] |= 1;

                //Now update the current page directory
                PD_Entry *pd = (PD_Entry*)GET_ADDR(&curInstance_virt[pdpt_i]);

                if(!pd[pd_i].present)
                {
                        uint32_t addr = virtMemMan_GetFreePageDirEntry();
                        memset(addr, 0, KB(4));
                        pd[pd_i].addr = ((uint32_t)addr/KB(4));
                        pd[pd_i].read_write = TRUE;
                        pd[pd_i].user_supervisor = privLevel;
                        pd[pd_i].accessed = 0;
                        pd[pd_i].present = 1;
                        pd[pd_i].nx = 0;
                        pd[pd_i].res2 = 0;
                        pd[pd_i].res1 = 0;
                        pd[pd_i].write_through = 0;
                        pd[pd_i].cache_disable = 0;

                }

                PT_Entry *pt = (PT_Entry*)(pd[pd_i].addr * KB(4));

                pt[pt_i].addr = physAddr/KB(4);
                pt[pt_i].present = 1;
                pt[pt_i].user_supervisor = privLevel;
                pt[pt_i].read_write = (perms & MEM_WRITE == MEM_WRITE);
                pt[pt_i].nx = (perms & MEM_EXEC  != MEM_EXEC);

                //Setup cache controls
                pt[pt_i].global = 0;
                pt[pt_i].pat = 0;
                pt[pt_i].write_through = (uint32_t)type & 1;
                pt[pt_i].cache_disable = ((uint32_t)type >> 1) & 1;

                //Flush the TLB
                asm volatile ("invlpg (%0)" :: "r" (virtAddr));
        }else
        {
                int64_t size_c = size;
                //Break down all uneven sized allocations into page sized ones
                while(size_c > 0)
                {
                        if(pt_i != 0 || (pt_i == 0 && size_c < MB(2)))
                        {
                                virtMemMan_Map(virtAddr, physAddr, KB(4), type, perms, privLevel);
                                size_c -= KB(4);
                                virtAddr += (uint32_t)KB(4);
                                physAddr += (uint64_t)KB(4);
                                pt_i++;
                        }
                        else if(pt_i == 0 && size_c >= MB(2))
                        {
                                virtMemMan_Map(virtAddr, physAddr, MB(2), type, perms, privLevel);
                                size_c -= MB(2);
                                virtAddr += (uint32_t)MB(2);
                                physAddr += MB(2);
                                pd_i++;
                        }

                        if(pt_i >= 512) {
                                pt_i = 0;
                                pd_i++;
                        }
                        if(pd_i >= 512)
                        {
                                pd_i = 0;
                                pdpt_i++;
                        }
                }
        }
        return 0;
}

void virtMemMan_UnMap(void* v_address, size_t size)
{
        if(size == 0) return;

        uint32_t virtAddr = (uint32_t)v_address;
        //Calculate the indices
        uint32_t pdpt_i = virtAddr/GB(1);
        uint32_t pd_i = (virtAddr - (pdpt_i * GB(1)))/MB(2);
        uint32_t pt_i = (virtAddr & 0x00000fff);

        //TODO this needs a clean rewrite

        if(size >= MB(2))
        {
                uint32_t seg_cnt = size/MB(2);
                if(seg_cnt == 0 || ((seg_cnt * MB(2)) < size)) seg_cnt++;

                PD_Entry_PSE *pd_pse = (PD_Entry_PSE*)GET_ADDR(&curInstance_virt[pdpt_i]);

                for(int i = 0; i < seg_cnt && pd_i + i < 512; i++)
                {
                        pd_pse[pd_i + i].present = 0;
                        pd_pse[pd_i + i].addr = 0;

                        //Flush the TLB
                        asm volatile ("invlpg (%0)" :: "r" ( (pdpt_i * GB(1)) + (pd_i + i)*MB(2)));
                }
        }else{
                //TODO Add 4KB page support
                uint32_t seg_cnt = size/KB(4);
                if(seg_cnt == 0 || ((seg_cnt * KB(4)) < size)) seg_cnt++;

                PD_Entry_PSE *pd_pse = (PD_Entry_PSE*)GET_ADDR(&curInstance_virt[pdpt_i]);
                uint64_t *pd_u64 = (uint64_t*)GET_ADDR(&curInstance_virt[pdpt_i]);

                if(pd_pse[pd_i].page_size == 0)
                {
                        PT_Entry *pt = (PT_Entry*)(pd_u64[pd_i] & 0xfffff000);

                        for(int i = 0; i < seg_cnt && pt_i + i < 512; i++)
                        {
                                pt[pt_i + i].present = 0;
                                pt[pt_i + i].addr = 0;
                        }
                }else{
                        //TODO break the page into a page table
                }
        }
}

uint64_t* virtMemMan_GetFreePDPTEntry()
{
        for(uint32_t i = 0; i < PDPT_STORAGE_SIZE_U64; i += 4)
        {
                if(pdpt_storage[i] == 0xFFFFFFFFFFFFFFFF) return &pdpt_storage[i];
        }
        return -1;
}

PD_Entry_PSE* virtMemMan_GetFreePageDirEntry()
{
        for(uint32_t i = 0; i < PAGE_DIR_STORAGE_POOL_SIZE; i+=KB(4))
        {
                if( *(uint64_t*)&page_dir_storage[i] == 0xFFFFFFFFFFFFFFFF) return (PD_Entry_PSE*)&page_dir_storage[i];
        }
        return -1;
}


void virtMemMan_FreeInstance(VirtMemMan_Instance inst)
{
        //Make sure this isn't current
        if(inst != curInstance_virt)
        {

                for(int j = 1; j < 4; j++)
                {
                        PD_Entry_PSE *pd_pse = (PD_Entry_PSE*)GET_ADDR(&curInstance_virt[j]);
                        uint64_t *pd_vals = (uint64_t*)GET_ADDR(&curInstance_virt[j]);
                        for(int i = 0; i < 512; i++)
                        {
                                if(pd_pse[i].present && pd_pse[i].page_size == 0)
                                {
                                        //Free the page table too
                                        memset((void*)(pd_vals[i] & 0xFFFFF000), 0xFF, KB(4));
                                }
                        }
                }

                memset((void*)inst[1], 0xFF, KB(4));
                memset((void*)inst[2], 0xFF, KB(4));
                memset((void*)inst[3], 0xFF, KB(4));
                memset((void*)inst, 0xFF, sizeof(uint64_t) * 4);
        }
}
