#ifndef _PRIV_VIRT_MEM_MANAGER_H_
#define _PRIV_VIRT_MEM_MANAGER_H_

#include "types.h"
#include "virt_mem_manager.h"
#include "managers.h"

#define KMEM_START 0
#define KMEM_END 0x40000000
#define KMEM_SIZE (KMEM_END-KMEM_START)

#define PAT_MSR 0x277

//Page Directory Pointer Table Entry
typedef struct
{
    uint64_t present : 1;
    uint64_t res0 : 2;
    uint64_t write_through : 1;
    uint64_t cache_disable : 1;
    uint64_t res1 : 7;
    uint64_t addr : 40; //Watch out! This is addr we want >> 12
    uint64_t res2 : 12;
} PDPT_Entry;

//Page Directory Entry
typedef struct
{
    uint64_t present : 1;
    uint64_t read_write : 1;
    uint64_t user_supervisor : 1;
    uint64_t write_through : 1;
    uint64_t cache_disable : 1;
    uint64_t accessed : 1;
    uint64_t res1 : 6;
    uint64_t addr : 40; //Watch out! This is addr we want >> 12
    uint64_t res2 : 11;
    uint64_t nx : 1;
} PD_Entry;

//Page Directory Entry 2MB pages
typedef struct
{
    uint64_t present : 1;
    uint64_t read_write : 1;
    uint64_t user_supervisor : 1;
    uint64_t write_through : 1;
    uint64_t cache_disable : 1;
    uint64_t accessed : 1;
    uint64_t dirty : 1;
    uint64_t page_size : 1;
    uint64_t global : 1;
    uint64_t res1 : 3;
    uint64_t pat : 1;
    uint64_t res2 : 8;
    uint64_t addr : 42; //Watch out! This is addr we want >> 12
    uint64_t nx : 1;
} PD_Entry_PSE;

//Page Table Entry
typedef struct
{
    uint64_t present : 1;
    uint64_t read_write : 1;
    uint64_t user_supervisor : 1;
    uint64_t write_through : 1;
    uint64_t cache_disable : 1;
    uint64_t accessed : 1;
    uint64_t dirty : 1;
    uint64_t pat : 1;
    uint64_t global : 1;
    uint64_t res1 : 3;
    uint64_t addr : 40; //Watch out! This is addr we want >> 12
    uint64_t res2 : 11;
    uint64_t nx : 1;
} PT_Entry;

#define GET_ADDR(x) (((PDPT_Entry*)x)->addr << 12)
#define SET_ADDR(val) (((uint64_t)(val)) >> 12)

#define PDPT_STORAGE_SIZE_U64  ((PAGE_DIR_STORAGE_POOL_SIZE/KB(12)) * 4)

static uint64_t* 
virtMemMan_GetFreePDPTEntry(void);

static PD_Entry_PSE* 
virtMemMan_GetFreePageDirEntry(void);

static uint32_t 
virtMemMan_PageFaultHandler(Registers *regs);

#endif /* end of include guard: _PRIV_VIRT_MEM_MANAGER_H_ */
