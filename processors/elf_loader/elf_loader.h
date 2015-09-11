#ifndef _ELF_LOADER_PROC_H_
#define _ELF_LOADER_PROC_H_

#include "types.h"

UID Elf_Load(const char *path);
void Elf_Start(UID id);

#endif