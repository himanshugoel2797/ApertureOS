#ifndef _ELF_LOADER_PROC_H_
#define _ELF_LOADER_PROC_H_

#include "types.h"

typedef enum{
	ELF_USER = 0,
	ELF_KERNEL = 1
}ELF_FLAGS;

UID Elf_Load(const char *path, ELF_FLAGS perms);
void Elf_Start(UID id);

#endif