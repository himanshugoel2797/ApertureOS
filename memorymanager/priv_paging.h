#ifndef _PRIV_PAGING_H_
#define _PRIV_PAGING_H_

#include <stddef.h>
#include <stdint.h>

//Page Directory Entry
typedef struct {
        uint32_t present : 1;
        uint32_t read_write : 1;
        uint32_t user_supervisor : 1;
        uint32_t write_through : 1;
        uint32_t cache_disable : 1;
        uint32_t accessed : 1;
        uint32_t dirty : 1;
        uint32_t large_page : 1;
        uint32_t res1 : 4;
        uint32_t pat : 1;
        uint32_t high_addr : 4;
        uint32_t res2 : 5;
        uint32_t low_addr : 10;
}PD_Entry_PSE;

typedef struct {
        uint32_t present : 1;
        uint32_t read_write : 1;
        uint32_t user_supervisor : 1;
        uint32_t write_through : 1;
        uint32_t cache_disable : 1;
        uint32_t accessed : 1;
        uint32_t dirty : 1;
        uint32_t large_page : 1;
        uint32_t res1 : 4;
        uint32_t addr : 20;
}PD_Entry_NOPSE;

//Page Table Entry
typedef struct {
        uint32_t present : 1;
        uint32_t read_write : 1;
        uint32_t user_supervisor : 1;
        uint32_t write_through : 1;
        uint32_t cache_disable : 1;
        uint32_t accessed : 1;
        uint32_t dirty : 1;
        uint32_t pat : 1;
        uint32_t global : 1;
        uint32_t res1 : 3;
        uint32_t addr : 20;
}PT_Entry;

#define SET_ADDR(dest, a) ((dest).addr = ((uint32_t)(a) >> 12))
#define GET_ADDR(src) ((src).addr << 12)
#define GET_PSE(n) ((PD_Entry_PSE)n)

#define PT_Entry_INIT(entry) \
        entry.present = 0; \
        entry.read_write = 1; \
        entry.user_supervisor = 0; \
        entry.write_through = 0; \
        entry.cache_disable = 0; \
        entry.accessed = 0; \
        entry.dirty = 0; \
        entry.pat = 0; \
        entry.global = 0; \
        entry.addr = 0;


#define PD_NO_PSE_Entry_INIT(entry) \
        entry.present = 0; \
        entry.read_write = 1; \
        entry.user_supervisor = 0; \
        entry.write_through = 0; \
        entry.cache_disable = 0; \
        entry.accessed = 0; \
        entry.dirty = 0; \
        entry.large_page = 0; \
        entry.res1 = 0; \
        entry.addr = 0;


typedef struct {
        uint32_t offset : 12;
        uint32_t table_index : 10;
        uint32_t directory_index : 10;
}KB4_t;

typedef struct {
        uint32_t MB4_PAGE_offset : 22;
        uint32_t directory_index : 10;
}MB4_t;

typedef union {
        KB4_t KB4;
        MB4_t MB4;
}VIRTUAL_ADDR;

//Page directory pointer table
PD_Entry_NOPSE *pd_nopse;
PD_Entry_PSE *pd_pse;

#endif /* end of include guard: _PRIV_PAGING_H_ */
