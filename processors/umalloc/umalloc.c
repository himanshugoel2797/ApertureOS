#include "umalloc.h"
#include "managers.h"
#include "utils/common.h"
#include "utils/native.h"

typedef struct umalloc_info
{
    uint32_t pointer;
    uint32_t size;
    struct umalloc_info *next;
} umalloc_info;


void* u_pages_base_addr;
int u_max_allocs = 0;
uint32_t u_free_space = 0;
umalloc_info *u_allocation_info = NULL, *u_next_free_block = NULL;

//A block is free if the first bit is clear
#define IS_FREE(x) ((x->pointer & 1) == 0)
#define IS_USED(x) ((x->pointer & 1) == 1)

#define MARK_FREE(x) (x->pointer = CLEAR_BIT(x->pointer, 0))
#define MARK_USED(x) (x->pointer = SET_BIT(x->pointer, 0))

#define GET_ADDR(x) (x->pointer & ~1)
#define SET_ADDR(x, val) (x->pointer = val | (x->pointer & 1))

// Allocate a set of pages and maintain a list of them, each page contains a
// umalloc_page_hdr header to track how much of the page is in use, kernel
// RAM
// is from virtual addresses 0x20000000 to 0x40000000 without regard for the
// associated physical page address

//Allocate a 256MB pool for the kernel and map it to a free address space
void umalloc_init()
{
#define STORE_SIZE MB(128)

    //Allocate blocks of 4KB and map them to a continuous address space of 256MB
    uint32_t virtBaseAddr_base = virtMemMan_FindEmptyAddress(STORE_SIZE, MEM_USER);
    size_t size = STORE_SIZE;
    while(size > 0)
    {
        uint64_t physBaseAddr_base = physMemMan_Alloc();
        virtMemMan_Map(virtBaseAddr_base, physBaseAddr_base, KB(4), MEM_TYPE_WB, MEM_READ | MEM_WRITE, MEM_USER);
        virtBaseAddr_base += KB(4);
        size -= KB(4);
    }
    virtBaseAddr_base -= STORE_SIZE;

    u_next_free_block = u_allocation_info = virtBaseAddr_base;
    u_pages_base_addr = virtBaseAddr_base + MB(1);
    u_max_allocs = MB(1)/sizeof(umalloc_info);
    u_free_space = STORE_SIZE - MB(1);

    memset(u_allocation_info, 0, MB(1));
    u_allocation_info->pointer = u_pages_base_addr;
    u_allocation_info->size = u_free_space;
    u_allocation_info->next = NULL;
    MARK_FREE(u_allocation_info);

    u_next_free_block++;
}

void ucompact()
{
    Interrupts_Lock();
    umalloc_info *a_info = u_allocation_info;

    while(a_info->next != NULL)
    {
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
    Interrupts_Unlock();
}

bool u_retry = FALSE;
void *umalloc(size_t size)
{
    umalloc_info *a_info = u_allocation_info;
    while(a_info != NULL && a_info->next != NULL)
    {
        if(IS_FREE(a_info) && a_info->size >= size)
        {
            break;
        }
        a_info = a_info->next;
    }


    if(IS_USED(a_info) | (a_info->size < size))
    {
        //Compact the allocation info and try again, if failed, return NULL
        if(!u_retry)
        {
            u_retry = TRUE;
            //kcompact();
            uint32_t res = umalloc(size);
            u_retry = FALSE;
            return res;
        }
        return NULL;
    }


    //Allocate this block, mark this one as used, append a new block object at the end that contains the remaining free space
    uint32_t addr = GET_ADDR(a_info);
    size_t freeSize = a_info->size - size;

    //We need to allocate a new info block only if there is free space
    if(freeSize != 0)
    {
        u_next_free_block->pointer = addr + size;
        u_next_free_block->size = freeSize;
        u_next_free_block->next = a_info->next;
        MARK_FREE(u_next_free_block);

        a_info->next = u_next_free_block;
        u_next_free_block++;

    }
    MARK_USED(a_info);
    a_info->size = size;
    //TODO redesign this to automatically request more space when necessary
    return addr;
}

void ufree(void *addr)
{
    //Find the block that matches the address specified
    umalloc_info *a_info = u_allocation_info;
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
