#ifndef _VIRT_MEM_MAN_H_
#define _VIRT_MEM_MAN_H_

#include "types.h"

typedef enum {
        MEM_TYPE_UC,
        MEM_TYPE_WB,
        MEM_TYPE_WT,
        MEM_TYPE_WC
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

typedef struct {
        uint64_t pdpt[4];
}VirtMemMan_Instance;

void virtMemMan_Setup();
void virtMemMan_SetCurrent(VirtMemMan_Instance *instance);
VirtMemMan_Instance* virtMemMan_GetCurrent();
void virtMemMan_CreateInstance(VirtMemMan_Instance *instance);
void virtMemMan_Fork(VirtMemMan_Instance *dst, VirtMemMan_Instance *src);
void virtMemMan_ForkCurrent(VirtMemMan_Instance *dst);
void* virtMemMan_FindEmptyAddress(size_t size, MEM_SECURITY_PERMS privLevel);
void virtMemMan_Map(void* v_address, void* phys_address, size_t size, MEM_TYPES type, MEM_ACCESS_PERMS perms, MEM_SECURITY_PERMS privLevel);
void virtMemMan_UnMap(void* v_address, size_t size);

#endif /* end of include guard: _VIRT_MEM_MAN_H_ */
