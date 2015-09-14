#ifndef _PRIV_ELF_LOADER_PROC_H_
#define _PRIV_ELF_LOADER_PROC_H_

#include "managers.h"
#include "elf.h"
#include "types.h"

typedef enum{
	ELF_USER = 0,
	ELF_KERNEL = 1
}ELF_FLAGS;

typedef struct ELF_Info_T
{
    UID id;
    void (*elf_main)(void);
    ELF_FLAGS flags;
    struct ELF_Info_T *next;
} ELF_Info;


#endif