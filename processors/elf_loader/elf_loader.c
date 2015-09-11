#include "elf_loader.h"
#include "priv_elf_loader.h"

UID e_uid_base = 0;
ELF_Info *e_info = NULL;

UID Elf_Load(const char *path)
{
	//Open the file
	UID fd = Filesystem_OpenFile(path, O_RDONLY, 0777);

	if(fd <= 0)return -1;

	//Filesystem_ReadFile(fd, )
}