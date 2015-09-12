#ifndef _ELF_H_
#define _ELF_H_

#include "types.h"

typedef uint16_t Elf32_Half;  // Unsigned half int
typedef uint32_t Elf32_Off; // Unsigned offset
typedef uint32_t Elf32_Addr;  // Unsigned address
typedef uint32_t Elf32_Word;  // Unsigned int
typedef int32_t Elf32_Sword;  // Signed int

#define ELF_NIDENT  16

typedef struct
{
    uint8_t e_ident[ELF_NIDENT];
    Elf32_Half e_type;
    Elf32_Half e_machine;
    Elf32_Word e_version;
    Elf32_Addr e_entry;
    Elf32_Off e_phoff;
    Elf32_Off e_shoff;
    Elf32_Word e_flags;
    Elf32_Half e_ehsize;
    Elf32_Half e_phentsize;
    Elf32_Half e_phnum;
    Elf32_Half e_shentsize;
    Elf32_Half e_shnum;
    Elf32_Half e_shstrndx;
} Elf32_Ehdr;

enum Elf_Ident
{
    EI_MAG0   = 0,// 0x7F
    EI_MAG1   = 1,// 'E'
    EI_MAG2   = 2,// 'L'
    EI_MAG3   = 3,// 'F'
    EI_CLASS  = 4,// Architecture (32/64)
    EI_DATA   = 5,// Byte Order
    EI_VERSION  = 6,// ELF Version
    EI_OSABI  = 7,// OS Specific
    EI_ABIVERSION = 8, // OS Specific
    EI_PAD    = 9// Padding
};

#define ELFMAG0 0x7F // e_ident[EI_MAG0]
#define ELFMAG1  'E'  // e_ident[EI_MAG1]
#define ELFMAG2  'L'  // e_ident[EI_MAG2]
#define ELFMAG3  'F'  // e_ident[EI_MAG3]
#define ELFDATA2LSB  (1)  // Little Endian
#define ELFCLASS32 (1)  // 32-bit Architecture

enum Elf_Type
{
    ET_NONE   = 0,// Unkown Type
    ET_REL    = 1,// Relocatable File
    ET_EXEC   = 2,// Executable File
};

enum Ph_Type
{
    PH_LOAD = 1,
    PH_SCE_RELOCATE = 0x60000000
};

enum Pf_Type
{
    PF_X = 1,
    PF_W = 2,
    PF_R = 4
};

#define EM_x86      (0x3)  // x86 Machine Type
#define EV_CURRENT  (1)  // ELF Current Version

typedef struct
{
    Elf32_Word p_type;
    Elf32_Off p_offset;
    Elf32_Addr p_vaddr;
    Elf32_Addr p_paddr;
    Elf32_Word p_filesz;
    Elf32_Word p_memsz;
    Elf32_Word p_flags;
    Elf32_Word p_align;
} Elf32_Phdr;

typedef struct
{
    Elf32_Word sh_name;
    Elf32_Word sh_type;
    Elf32_Word sh_flags;
    Elf32_Addr sh_addr;
    Elf32_Off sh_offset;
    Elf32_Word sh_size;
    Elf32_Word sh_link;
    Elf32_Word sh_info;
    Elf32_Word sh_addralign;
    Elf32_Word sh_entsize;
} Elf32_Shdr;

typedef struct
{
    Elf32_Word st_name;
    Elf32_Addr st_value;
    Elf32_Word st_size;
    uint8_t st_info;
    uint8_t st_other;
    Elf32_Half st_shndx;
} Elf32_Sym;


#endif /* end of include guard: _ELF_H_ */
