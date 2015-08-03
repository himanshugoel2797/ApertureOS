#ifndef _MULTIBOOT_HEADER_H_
#define _MULTIBOOT_HEADER_H_

typedef unsigned short multiboot_uint16_t;
typedef unsigned int multiboot_uint32_t;
typedef unsigned long long multiboot_uint64_t;

typedef struct
   {
      unsigned short setWindow;
      unsigned short setDisplayStart;
      unsigned short setPalette;
      unsigned short IOPrivInfo;
   } __attribute__((packed)) VESA_PM_INFO;

typedef struct {
   char VbeSignature[4];             // == "VESA"
   uint16_t VbeVersion;                 // == 0x0300 for VBE 3.0
   uint16_t OemStringPtr[2];            // isa vbeFarPtr
   uint8_t Capabilities[4];
   uint16_t VideoModePtr[2];         // isa vbeFarPtr
   uint16_t TotalMemory;             // as # of 64KB blocks
} __attribute__((packed)) VbeInfoBlock;

typedef struct {
  uint16_t attributes;
  uint8_t winA,winB;
  uint16_t granularity;
  uint16_t winsize;
  uint16_t segmentA, segmentB;
  void (*realFctPtr)();
  uint16_t pitch; // bytes per scanline

  uint16_t Xres, Yres;
  uint8_t Wchar, Ychar, planes, bpp, banks;
  uint8_t memory_model, bank_size, image_pages;
  uint8_t reserved0;

  uint8_t red_mask, red_position;
  uint8_t green_mask, green_position;
  uint8_t blue_mask, blue_position;
  uint8_t rsv_mask, rsv_position;
  uint8_t directcolor_attributes;

  uint32_t physbase;  // your LFB (Linear Framebuffer) address ;)
  uint32_t offScrOffset;
  uint16_t offScrSize;
} __attribute__((packed)) ModeInfoBlock;

struct multiboot_header
{
        /* Must be MULTIBOOT_MAGIC - see above. */
        multiboot_uint32_t magic;

        /* Feature flags. */
        multiboot_uint32_t flags;

        /* The above fields plus this one must equal 0 mod 2^32. */
        multiboot_uint32_t checksum;

        /* These are only valid if MULTIBOOT_AOUT_KLUDGE is set. */
        multiboot_uint32_t header_addr;
        multiboot_uint32_t load_addr;
        multiboot_uint32_t load_end_addr;
        multiboot_uint32_t bss_end_addr;
        multiboot_uint32_t entry_addr;

        /* These are only valid if MULTIBOOT_VIDEO_MODE is set. */
        multiboot_uint32_t mode_type;
        multiboot_uint32_t width;
        multiboot_uint32_t height;
        multiboot_uint32_t depth;
};

/* The symbol table for a.out. */
struct multiboot_aout_symbol_table
{
        multiboot_uint32_t tabsize;
        multiboot_uint32_t strsize;
        multiboot_uint32_t addr;
        multiboot_uint32_t reserved;
};
typedef struct multiboot_aout_symbol_table multiboot_aout_symbol_table_t;

/* The section header table for ELF. */
struct multiboot_elf_section_header_table
{
        multiboot_uint32_t num;
        multiboot_uint32_t size;
        multiboot_uint32_t addr;
        multiboot_uint32_t shndx;
};
typedef struct multiboot_elf_section_header_table multiboot_elf_section_header_table_t;

struct multiboot_info
{
        /* Multiboot info version number */
        multiboot_uint32_t flags;

        /* Available memory from BIOS */
        multiboot_uint32_t mem_lower;
        multiboot_uint32_t mem_upper;

        /* "root" partition */
        multiboot_uint32_t boot_device;

        /* Kernel command line */
        multiboot_uint32_t cmdline;

        /* Boot-Module list */
        multiboot_uint32_t mods_count;
        multiboot_uint32_t mods_addr;

        union
        {
                multiboot_aout_symbol_table_t aout_sym;
                multiboot_elf_section_header_table_t elf_sec;
        } u;

        /* Memory Mapping buffer */
        multiboot_uint32_t mmap_length;
        multiboot_uint32_t mmap_addr;

        /* Drive Info buffer */
        multiboot_uint32_t drives_length;
        multiboot_uint32_t drives_addr;

        /* ROM configuration table */
        multiboot_uint32_t config_table;

        /* Boot Loader Name */
        multiboot_uint32_t boot_loader_name;

        /* APM table */
        multiboot_uint32_t apm_table;

        /* Video */
        VbeInfoBlock* vbe_control_info;
        ModeInfoBlock* vbe_mode_info;
        multiboot_uint16_t vbe_mode;
        multiboot_uint16_t vbe_interface_seg;
        multiboot_uint16_t vbe_interface_off;
        multiboot_uint16_t vbe_interface_len;
};
typedef struct multiboot_info multiboot_info_t;

struct multiboot_mmap_entry
{
        multiboot_uint32_t size;
        multiboot_uint64_t addr;
        multiboot_uint64_t len;
     #define MULTIBOOT_MEMORY_AVAILABLE              1
     #define MULTIBOOT_MEMORY_RESERVED               2
        multiboot_uint32_t type;
} __attribute__((packed));
typedef struct multiboot_mmap_entry multiboot_memory_map_t;

struct multiboot_mod_list
{
        /* the memory used goes from bytes 'mod_start' to 'mod_end-1' inclusive */
        multiboot_uint32_t mod_start;
        multiboot_uint32_t mod_end;

        /* Module command line */
        multiboot_uint32_t cmdline;

        /* padding to take it to 16 bytes (must be zero) */
        multiboot_uint32_t pad;
};
typedef struct multiboot_mod_list multiboot_module_t;

#endif /* end of include guard: _MULTIBOOT_HEADER_H_ */
