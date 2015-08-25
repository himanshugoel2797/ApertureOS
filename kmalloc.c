#include "kmalloc.h"
#include "managers.h"

typedef struct kmalloc_info {

        uint32_t pointer;
        size_t size;
        struct kmalloc_info *next;
} kmalloc_info;


void* k_pages_base_addr;
int max_allocs = 0;
uint32_t free_space = 0;
kmalloc_info *allocation_info;

//A block is free if the first bit is clear
#define IS_FREE(x) (!((allocation_info[x].pointer & 1) == 1))
#define IS_USED(x) (IS_FREE(x))

// Allocate a set of pages and maintain a list of them, each page contains a
// kmalloc_page_hdr header to track how much of the page is in use, kernel
// RAM
// is from virtual addresses 0x20000000 to 0x40000000 without regard for the
// associated physical page address

//Allocate a 256MB pool for the kernel and map it to a free address space
void kmalloc_init()
{
        uint64_t physBaseAddr_base = physMemMan_Alloc(MB(256));

        uint32_t virtBaseAddr_base = virtMemMan_FindEmptyAddress(MB(256), MEM_KERNEL);
        virtMemMan_Map(virtBaseAddr_base, physBaseAddr_base, MB(256), MEM_TYPE_WC, MEM_READ | MEM_WRITE, MEM_KERNEL);

        allocation_info = virtBaseAddr_base;
        k_pages_base_addr = virtBaseAddr_base + MB(1);
        max_allocs = MB(1)/sizeof(kmalloc_info);
        free_space = MB(256) - MB(1);

        memset(allocation_info, 0, MB(1));
        allocation_info->pointer = k_pages_base_addr;
        allocation_info->size = free_space;
}

void *kmalloc(size_t size)
{
        kmalloc_info *a_info = allocation_info;
        while(a_info->next != NULL)
        {
                if(a_info->size >= size)
                {
                        break;
                }
                a_info = a_info->next;
        }
}

void kfree(void *addr)
{

}
