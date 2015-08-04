#include <stddef.h>
#include <stdint.h>

#include "gdt.h"

//Describes a single GDT entry
typedef struct {
        uint16_t limit_low;
        uint16_t base_low;
        uint8_t base_mid;

        uint8_t access;
        uint8_t granularity;

        uint8_t base_high;

}__attribute__((packed)) GDTEntry;

void GDT_SetEntry(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran);

//Describes the pointer to the GDT entry
typedef struct {
        uint16_t limit; //Size
        uint32_t base; //Location
}__attribute__((packed)) GDTPtr;

//GDT Entry region
GDTEntry gdt_entries[GDT_ENTRY_COUNT];
GDTPtr gdt_table;

__attribute__((optnone))
void GDT_Initialize()
{
        //Make sure interrupts are disabled
        asm ("cli");

        gdt_table.limit = (sizeof(GDTEntry) * GDT_ENTRY_COUNT) - 1;
        gdt_table.base = (uint32_t)&gdt_entries;

        GDT_SetEntry(0, 0, 0, 0, 0);
        GDT_SetEntry(1, 0, 0xFFFFFFFF, 0x9A, 0xCF); // Code segment
        GDT_SetEntry(2, 0, 0xFFFFFFFF, 0x92, 0xCF); // Data segment
        GDT_SetEntry(3, 0, 0xFFFFFFFF, 0xFA, 0xCF); // User mode code segment
        GDT_SetEntry(4, 0, 0xFFFFFFFF, 0xF2, 0xCF); // User mode data segment

        asm ("lgdt (%0)" :: "r" (&gdt_table));

        asm (
                "jmp $0x8, $flush\n\t"
                "flush:\n\t"
                "mov $0x10, %ax\n\t"
                "mov %ax, %ds\n\t"
                "mov %ax, %es\n\t"
                "mov %ax, %fs\n\t"
                "mov %ax, %gs\n\t"
                "mov %ax, %ss\n\t"
                );
        return; //Don't enable interrupts yet
}

void GDT_SetEntry(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran)
{
        gdt_entries[num].base_low    = (base & 0xFFFF);
        gdt_entries[num].base_mid = (base >> 16) & 0xFF;
        gdt_entries[num].base_high   = (base >> 24) & 0xFF;

        gdt_entries[num].limit_low   = (limit & 0xFFFF);
        gdt_entries[num].granularity = (limit >> 16) & 0x0F;

        gdt_entries[num].granularity |= gran & 0xF0;
        gdt_entries[num].access     = access;
}
