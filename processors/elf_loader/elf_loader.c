#include "elf_loader.h"
#include "priv_elf_loader.h"
#include "kmalloc.h"
#include "utils/common.h"

ELF_Info *e_info = NULL, *last_e_info = NULL;

UID
Elf_Load(const char *path, ELF_FLAGS perms)
{
    //Open the file
    UID fd = Filesystem_OpenFile(path, O_RDONLY, 0777);
    if(fd <= 0)return -1;

    //Get the file length
    uint32_t file_size = Filesystem_SeekFile(fd, 0, SEEK_END);
    COM_WriteStr("STARTING\r\n");
    Filesystem_SeekFile(fd, 0, SEEK_SET);

    uint8_t *elf_temp = kmalloc(file_size);
    Filesystem_ReadFile(fd, elf_temp, file_size);


    //Parse the elf from here
    Elf32_Ehdr *hdr = (Elf32_Ehdr*)elf_temp;
    if(hdr->e_ident[EI_MAG0] != ELFMAG0)goto error;
    if(hdr->e_ident[EI_MAG1] != ELFMAG1)goto error;
    if(hdr->e_ident[EI_MAG2] != ELFMAG2)goto error;
    if(hdr->e_ident[EI_MAG3] != ELFMAG3)goto error;
    if(hdr->e_ident[EI_CLASS]!= ELFCLASS32)goto error;
    if(hdr->e_ident[EI_DATA] != ELFDATA2LSB)goto error;
    if(hdr->e_ident[EI_VERSION] != EV_CURRENT)goto error;
    if(hdr->e_ident[EI_OSABI] != 0)goto error;
    if(hdr->e_machine != EM_x86)goto error;

    //Don't support relocations for now
    if(hdr->e_type == ET_EXEC)
        {
            Elf32_Phdr *phdr = (Elf32_Phdr*)(elf_temp + hdr->e_phoff);	//Find the program header

            for(int i = 0; i < hdr->e_phnum; i++)
                {
                    if(phdr->p_type == 1)
                        {

                            //Allocate the associated physical memory and map it into the address space
                            uint32_t page_count = (phdr->p_memsz - 1)/KB(4) + 1;
                            uint64_t p_addr = 0;
                            uint32_t v_addr = phdr->p_vaddr;

                            for(int j = 0; j < page_count; j++)
                                {
                                    p_addr = physMemMan_Alloc();
                                    virtMemMan_Map(v_addr, p_addr, KB(4), MEM_TYPE_WB, phdr->p_flags, MEM_USER);

                                    v_addr += KB(4);
                                }

                            v_addr = phdr->p_vaddr;
                            memset(v_addr, 0, phdr->p_memsz);
                            memcpy(v_addr, elf_temp + phdr->p_offset, phdr->p_filesz);
                            COM_WriteStr("TEST:%x\r\n", file_size);

                        }

                    phdr = (Elf32_Phdr*)((uint8_t*)phdr + hdr->e_phentsize);
                }

        }
    else goto error;

    kfree(elf_temp);

    ELF_Info *elf_info_tmp = kmalloc(sizeof(ELF_Info));
    elf_info_tmp->id = new_uid();
    elf_info_tmp->elf_main = (void(*)(void))hdr->e_entry;
    elf_info_tmp->flags = perms;

    if(e_info == NULL)
        {
            e_info = elf_info_tmp;
            last_e_info = e_info;
        }
    else
        {
            last_e_info->next = elf_info_tmp;
            last_e_info = last_e_info->next;
        }

    return elf_info_tmp->id;

error:
    COM_WriteStr("ERROR!!!");
    kfree(elf_temp);
    return -2;
}

void
Elf_Start(UID id)
{
    ELF_Info *inf = e_info;
    do
        {
            if(inf->id == id)break;
            inf = inf->next;
        }
    while(inf != NULL);

    if(inf == NULL)return;

    if(inf->flags & ELF_KERNEL == ELF_KERNEL)
        {
            inf->elf_main();
        }
    else
        {
            uint32_t x = 0x80000074;
            if((uint32_t)inf->elf_main == 0x80000000)x = 0x80000000;
            asm volatile(
                "cli \n\t"
                "mov $0x23, %%ax\n\t"
                "mov %%ax, %%ds\n\t"
                "mov %%ax, %%es\n\t"
                "mov %%ax, %%fs\n\t"
                "mov %%ax, %%gs\n\t"
                "push %%ebx\n\t"
                "mov %%esp, %%eax\n\t"
                "pushl $0x23\n\t"
                "pushl %%eax\n\t"
                "pushf\n\t"
                "pop %%eax\n\t"
                "or $512, %%eax\n\t"
                "push %%eax\n\t"
                "pushl $0x1B\n\t"
                "mov %%ebx, -0x14(%%esp)\n\t"
                "push %%ecx\n\t"
                "iret\n\t"
                :: "b"(inf->elf_main), "c"(x)
            );

        }

}