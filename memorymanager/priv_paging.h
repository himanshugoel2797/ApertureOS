#ifndef _PRIV_PAGING_H_
#define _PRIV_PAGING_H_

#include <stddef.h>
#include <stdint.h>

//Page Directory Pointer Table Entry
typedef struct {
        uint64_t present : 1;
        uint64_t res0 : 2;
        uint64_t write_through : 1;
        uint64_t cache_disable : 1;
        uint64_t res1 : 7;
        uint64_t addr : 40; //Watch out! This is addr we want >> 12
        uint64_t res2 : 12;
}PDPT_Entry;

//Page Directory Entry
typedef struct {
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
}PD_Entry;

//Page Table Entry
typedef struct {
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
}PT_Entry;

#define GET_ADDR(x) (x.addr << 12)
#define SET_ADDR(val) ((uint64_t)(val) >> 12)

#endif /* end of include guard: _PRIV_PAGING_H_ */