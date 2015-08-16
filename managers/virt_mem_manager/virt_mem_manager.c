#include "virt_mem_manager.h"
#include "priv_virt_mem_manager.h"
#include "managers.h"
#include "utils/common.h"

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
        memset(page_dir_storage, 0xFF, PAGE_DIR_STORAGE_POOL_SIZE);
        memset(pdpt_storage, 0xFF, PDPT_STORAGE_SIZE_U64 * sizeof(uint64_t));

        curInstance_virt = virtMemMan_GetFreePDPTEntry();
        memset(curInstance_virt, 0, sizeof(uint64_t) * 4);

        uint32_t pdpt_index = 0, pd_index = 0;
        for(uint32_t addr = 0; addr < 0x40000000; addr+=MB(2))
        {
                //Initialize the PDPT entry if it hasn't been initialized
                if(curInstance_virt[pdpt_index] == 0)
                {
                        curInstance_virt[pdpt_index] = (uint64_t)virtMemMan_GetFreePageDirEntry();
                        memset((void*)curInstance_virt[pdpt_index], 0, KB(4));

                        //Backup the main kernel page so it can be copied later
                        if(pdpt_index == 0) kernel_main_entry = (PD_Entry_PSE*)curInstance_virt[0];

                        curInstance_virt[pdpt_index] |= TRUE;
                        COM_WriteStr("\r\nPDPT: %x", GET_ADDR(&curInstance_virt[pdpt_index]));
                        COM_WriteStr("\r\nPDPA: %x", &curInstance_virt[pdpt_index]);
                        COM_WriteStr("\r\nPDPB: %x", &curInstance_virt);
                }

                PD_Entry_PSE* curPD = (PD_Entry_PSE*)GET_ADDR(&curInstance_virt[pdpt_index]);
                curPD[pd_index].addr = addr/MB(2);
                curPD[pd_index].present = TRUE;
                curPD[pd_index].read_write = 1;
                curPD[pd_index].page_size = 1;

                //Increment all counters
                pd_index++;
                if(pd_index == 512)
                {
                        pd_index = 0;
                        pdpt_index++;
                }
                if(pdpt_index == 4)
                {
                        break;
                }
        }


        //while(1) ;
        asm volatile ("movl %cr4, %eax; orl $0x00000010, %eax; movl %eax, %cr4;");  //Enable PSE
        asm volatile ("movl %cr4, %eax; bts $5, %eax; movl %eax, %cr4"); // set bit5 in CR4 to enable PAE
        asm volatile ("movl %%eax, %%cr3" :: "a" (curInstance_virt)); // load PDPT into CR3
        asm volatile ("movl %cr0, %eax; orl $0x80000000, %eax; movl %eax, %cr0;");  //Enable Paging
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
        if(seg_cnt == 0) seg_cnt++;

        if(privLevel == MEM_KERNEL)
        {
                //Search the kernel space to see if any virtual address slot is free
                for(int i = 0; i < 512; i++)
                {
                        if(kernel_main_entry[i].present == FALSE) {

                                int score = 0;
                                for(int j = 0; j < seg_cnt; j++) {
                                        if(kernel_main_entry[i+j].present == FALSE) score++;
                                }

                                if(score >= seg_cnt) return (i * MB(2));
                        }
                }
        }
        else if(privLevel == MEM_USER)
        {
                for(int i_pdpt = 1; i_pdpt < 4; i_pdpt++ )
                {
                        if((curInstance_virt[i_pdpt] & 1) == 1) {
                                PD_Entry_PSE *pd_pse = GET_ADDR(curInstance_virt[i_pdpt]);
                                for(int i_pd = 0; i_pd < 512; i_pd++)
                                {
                                        if(pd_pse[i_pd].present == FALSE) {

                                                //TODO this ought to be able to search across pdpt boundaries

                                                int score = 0;
                                                for(int j = 0; j < seg_cnt; j++)
                                                {
                                                        if(pd_pse[i_pd + j].present == FALSE) score++;
                                                }

                                                if(score >= seg_cnt) return (GB(1) * i_pdpt) + (MB(2) * i_pd);
                                        }
                                }
                        }else{
                                if(seg_cnt <= 512) return GB(1) * i_pdpt;
                        }
                }
        }
        return NULL;
}

void virtMemMan_Map(void* v_address, void* phys_address, size_t size, MEM_TYPES type, MEM_ACCESS_PERMS perms, MEM_SECURITY_PERMS privLevel)
{
        if(size == 0) return;

        uint32_t virtAddr = (uint32_t)v_address;
        uint64_t physAddr = (uint64_t)phys_address;
        uint32_t seg_cnt = size/MB(2);
        if(seg_cnt == 0) seg_cnt++;

        //Align the virtAddr to 2MB
        virtAddr = (virtAddr/MB(2)) * MB(2);

        //Calculate the indices
        uint32_t pdpt_i = virtAddr/GB(1);
        uint32_t pd_i = (virtAddr - (pdpt_i * GB(1)))/MB(2);

        //Check requested permissions to make sure they match up with the virtual address
        //Now update the current page directory
        //Setup the appropriate caching options
}

void virtMemMan_UnMap(void* v_address, size_t size)
{

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
                memset((void*)inst[1], 0xFF, KB(4));
                memset((void*)inst[2], 0xFF, KB(4));
                memset((void*)inst[3], 0xFF, KB(4));
                memset((void*)inst, 0xFF, sizeof(uint64_t) * 4);
        }
}
