#include "virt_mem_manager.h"
#include "priv_virt_mem_manager.h"
#include "managers.h"
#include "utils/common.h"

SystemData *vmem_sys = NULL;
uint32_t virtMemMan_Initialize();
void virtMemMan_callback(uint32_t res);
uint8_t virtMemMan_messageHandler(Message *msg);

VirtMemMan_Instance *curInstance_virt;

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
        curInstance_virt = physMemMan_Alloc(KB(4));
        memset(curInstance_virt, 0, sizeof(VirtMemMan_Instance));

        uint32_t pdpt_index = 0, pd_index = 0, pt_index = 0;
        for(uint32_t addr = 0; addr < -1 /*0x40000000*/; addr+=KB(4))
        {
                //Initialize the PDPT entry if it hasn't been initialized
                if(curInstance_virt->pdpt[pdpt_index] == 0)
                {
                        curInstance_virt->pdpt[pdpt_index] = (uint64_t)physMemMan_Alloc(KB(4));
                        memset((void*)curInstance_virt->pdpt[pdpt_index], 0, KB(4));

                        curInstance_virt->pdpt[pdpt_index] |= TRUE;
                }

                PD_Entry* curPD = (PD_Entry*)GET_ADDR(curInstance_virt->pdpt[pdpt_index]);
                if(!curPD[pd_index].present)
                {
                        curPD[pd_index].addr = SET_ADDR(physMemMan_Alloc(KB(4)));
                        memset(GET_ADDR(&curPD[pd_index]), 0, KB(4));

                        curPD[pd_index].present = TRUE;
                }

                PT_Entry* curPT = (PT_Entry*)GET_ADDR(&curPD[pd_index]);
                curPT[pt_index].addr = SET_ADDR(addr);
                curPT[pt_index].present = TRUE;

                //Increment all counters
                pt_index++;
                if(pt_index == 512) {
                        pt_index = 0;
                        pd_index++;
                        COM_WriteStr("PDPT: %d PD Entry: %d PT Entry: %d ADDR: %x REALADDR: %x\r\n", pdpt_index, pd_index, pt_index, addr, curPT[511].addr);
                }
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

        while(1) ;
        asm volatile ("movl %cr4, %eax; bts $5, %eax; movl %eax, %cr4"); // set bit5 in CR4 to enable PAE
        asm volatile ("movl %%eax, %%cr3" :: "a" (&curInstance_virt)); // load PDPT into CR3
        asm volatile ("movl %cr0, %eax; orl $0x80000000, %eax; movl %eax, %cr0;");
}

void virtMemMan_callback(uint32_t res)
{

}

uint8_t virtMemMan_messageHandler(Message *msg)
{

}

void virtMemMan_SetCurrent(VirtMemMan_Instance *instance)
{

}

VirtMemMan_Instance* virtMemMan_GetCurrent()
{
        return curInstance_virt;
}

void virtMemMan_CreateInstance(VirtMemMan_Instance *instance)
{
        //Identity map up to 0x40000000 assuming virtual memory is enabled
        PDPT_Entry *pdpt = instance->pdpt;


}

void virtMemMan_Fork(VirtMemMan_Instance *dst, VirtMemMan_Instance *src)
{

}

void virtMemMan_ForkCurrent(VirtMemMan_Instance *dst)
{

}

void* virtMemMan_FindEmptyAddress(size_t size, MEM_SECURITY_PERMS privLevel)
{

}

void virtMemMan_Map(void* v_address, void* phys_address, size_t size, MEM_TYPES type, MEM_ACCESS_PERMS perms, MEM_SECURITY_PERMS privLevel)
{

}

void virtMemMan_UnMap(void* v_address, size_t size)
{

}
