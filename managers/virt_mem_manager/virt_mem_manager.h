#ifndef _VIRT_MEM_MAN_H_
#define _VIRT_MEM_MAN_H_

#include "types.h"

#define PAGE_DIR_STORAGE_POOL_SIZE MB(18)
#define MEMIO_TOP_BASE 0xF0000000
#define VIRTUALIZE_HIGHER_MEM_OFFSET(a) (((uint32_t)a - 0xF0000000) + 0x10000000)

typedef enum {
        MEM_TYPE_UC = 2,
        MEM_TYPE_WB = 0,
        MEM_TYPE_WT = 1,
        MEM_TYPE_WC = 3
}MEM_TYPES;

typedef enum {
        MEM_WRITE = 1,
        MEM_READ = 2,
        MEM_EXEC = 4
}MEM_ACCESS_PERMS;

typedef enum {
        MEM_KERNEL = 0,
        MEM_USER = 1
}MEM_SECURITY_PERMS;

typedef uint64_t* VirtMemMan_Instance;

void virtMemMan_Setup();
VirtMemMan_Instance virtMemMan_SetCurrent(VirtMemMan_Instance *instance);
VirtMemMan_Instance virtMemMan_GetCurrent();
VirtMemMan_Instance virtMemMan_CreateInstance();
void virtMemMan_Fork(VirtMemMan_Instance dst, VirtMemMan_Instance src);
void virtMemMan_ForkCurrent(VirtMemMan_Instance dst);
void* virtMemMan_FindEmptyAddress(size_t size, MEM_SECURITY_PERMS privLevel);
uint32_t virtMemMan_Map(uint32_t v_address, uint64_t phys_address, size_t size, MEM_TYPES type, MEM_ACCESS_PERMS perms, MEM_SECURITY_PERMS privLevel);
void virtMemMan_UnMap(void* v_address, size_t size);
void virtMemMan_FreeInstance(VirtMemMan_Instance inst);

#endif /* end of include guard: _VIRT_MEM_MAN_H_ */
