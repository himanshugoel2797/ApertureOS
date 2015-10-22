#ifndef _PRIV_ELF_LOADER_PROC_H_
#define _PRIV_ELF_LOADER_PROC_H_

#include "managers.h"
#include "elf.h"
#include "types.h"
#include "elf_loader.h"

typedef struct ELF_Info_T
{
    UID id;
    void (*elf_main)(void);
    void (*initer)(void);
    ELF_FLAGS flags;
    struct ELF_Info_T *next;
} ELF_Info;


#endif