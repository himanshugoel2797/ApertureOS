#include "kmalloc.h"
#include "managers.h"
#include "utils/common.h"
#include "utils/native.h"

typedef struct kmalloc_info {
        uint32_t pointer;
        size_t size;
        struct kmalloc_info *next;
} kmalloc_info;


void* k_pages_base_addr;
int max_allocs = 0;
uint32_t free_space = 0;
kmalloc_info *allocation_info, *next_free_block;

//A block is free if the first bit is clear
#define IS_FREE(x) (!((x->pointer & 1) == 1))
#define IS_USED(x) (IS_FREE(x))

#define MARK_FREE(x) (x->pointer = CLEAR_BIT(x->pointer, 0))
#define MARK_USED(x) (x->pointer = SET_BIT(x->pointer, 0))

#define GET_ADDR(x) (x->pointer & ~1)
#define SET_ADDR(x, val) (x->pointer = val | (x->pointer & 1))

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

        next_free_block = allocation_info = virtBaseAddr_base;
        k_pages_base_addr = virtBaseAddr_base + MB(1);
        max_allocs = MB(1)/sizeof(kmalloc_info);
        free_space = MB(256) - MB(1);

        memset(allocation_info, 0, MB(1));
        allocation_info->pointer = k_pages_base_addr;
        allocation_info->size = free_space;

        next_free_block++;
}

void kcompact()
{
        interrupts_lock();
        kmalloc_info *a_info = allocation_info;

        while(a_info->next != NULL) {
                while(a_info->next != NULL)
                {
                        if(IS_FREE(a_info))
                        {
                                break;
                        }
                        a_info = a_info->next;
                }
                if(a_info->next != NULL)
                {
                        //TODO this is a memory leak, need some way to reclaim this memory
                        a_info->size += a_info->next->size;
                        a_info->next = a_info->next->next;
                        a_info = a_info->next;
                }
        }
        interrupts_unlock();
}

bool retry = FALSE;
void *kmalloc(size_t size)
{
        kmalloc_info *a_info = allocation_info;
        while(a_info->next != NULL)
        {
                if(IS_FREE(a_info) && a_info->size >= size)
                {
                        break;
                }
                a_info = a_info->next;
        }

        if(IS_USED(a_info) | a_info->size < size) {
                //Compact the allocation info and try again, if failed, return NULL
                if(!retry)
                {
                        retry = TRUE;
                        kcompact();
                        return kmalloc(size);
                        retry = FALSE;
                }
                return NULL;
        }

        //Allocate this block, mark this one as used, append a new block object at the end that contains the remaining free space
        uint32_t addr = GET_ADDR(a_info);
        uint32_t freeSize = a_info->size - size;

        //We need to allocate a new info block only if there is free space
        if(freeSize != 0)
        {
                next_free_block->pointer = addr + size;
                next_free_block->size = freeSize;
                next_free_block->next = a_info->next;
                MARK_FREE(next_free_block);

                a_info->next = next_free_block;
                next_free_block++;
        }

        a_info->size = size;
        MARK_USED(a_info);

        return addr;
}

void kfree(void *addr)
{
        //Find the block that matches the address specified
        kmalloc_info *a_info = allocation_info;
        while(a_info->next != NULL)
        {
                if(IS_USED(a_info) && a_info->pointer == (uint32_t)addr)
                {
                        break;
                }

                a_info = a_info->next;
        }

        //Mark this block as free
        MARK_FREE(a_info);



}
