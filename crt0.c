/*
 * File:   main.cpp
 * Author: himanshu
 *
 * Created on January 9, 2014, 7:52 PM
 */

#include <stddef.h>
#include <stdint.h>

#include "multiboot.h"
#include "elf.h"
#include "globals.h"

#include "gdt.h"
#include "idt.h"
#include "kmalloc.h"

#include "drivers/drivers.h"
#include "managers/managers.h"

#include "utils/native.h"
#include "utils/common.h"

#include "graphics/graphics.h"


int temp = 0, temp2 = 0;
char *tmp;


void t_main(int argc, char **argv);

//extern "C"{
void setup_kernel_core(multiboot_info_t* mbd, uint32_t magic)
{

    GDT_Initialize();
    IDT_Initialize();

    COM_Initialize();
    ACPITables_Initialize();

    bootstrap_setup();

    //Backup all important information from the bootloader
    global_vbe_info = bootstrap_malloc(sizeof(VbeInfoBlock));
    memcpy(global_vbe_info, mbd->vbe_control_info, sizeof(VbeInfoBlock));

    global_mode_info = bootstrap_malloc(sizeof(ModeInfoBlock));
    memcpy(global_mode_info, mbd->vbe_mode_info, sizeof(ModeInfoBlock));

    global_multiboot_info = bootstrap_malloc(sizeof(multiboot_info_t));
    memcpy(global_multiboot_info, mbd, sizeof(multiboot_info_t));
    global_memory_map_size = mbd->mmap_length;
    global_memory_map = bootstrap_malloc(global_memory_map_size);
    memcpy(global_memory_map, mbd->mmap_addr, global_memory_map_size);

    //All these need to access ACPI tables
    Interrupts_Setup();
    CMOS_Initialize();
    FPU_Initialize();
    pci_Initialize();

    //Once virtual memory management is put in place, ACPI tables become inaccessible, the acpi table will need to be copied
    physMemMan_Setup();
    virtMemMan_Setup();
    
    Interrupts_Virtualize();
    graphics_Initialize();
    
    //Attempt to initialize all PCI drivers here so they can mark their MMIO space as used
    AHCI_Initialize();

    kmalloc_init();
    Timers_Setup();
    ThreadMan_Setup();

    tmp = kmalloc(1080*1920*4 + KB(4));
    tmp += KB(4);
    tmp -= ((uint32_t)tmp) % KB(4);

    COM_WriteStr("Kernel Size: %x MiB\r\n", ((uint32_t)&_region_kernel_end_ - LOAD_ADDRESS)/(1024 * 1024));


    Keyboard_Setup();

    UID tid = ThreadMan_CreateThread( t_main, 50, 51, THREAD_FLAGS_USER);
    ThreadMan_StartThread(tid);

    Interrupts_Unlock();
    while(1);
}

void t_main(int argc, char **argv)
{

    Filesystem_Setup();

    UID fd = Filesystem_OpenFile("/home/hgoel/test.data", 0, 0);
    //uint8_t *buf = kmalloc(1920*1080*4 + KB(4));
    //buf += KB(4);
    //buf -= ((uint32_t)buf) % KB(4);
    //COM_WriteStr("%x\r\n", buf);

    Interrupts_Lock();
    COM_WriteStr("FD: %x\r\n", fd);
    memset(tmp, 0xff, 1920*1080*4);
    if(fd != -1)Filesystem_ReadFile(fd, tmp, 1920*1080*4);
    Interrupts_Unlock();
    //tmp = buf;

    UID id = Filesystem_OpenDir("/home/hgoel/");
    Filesystem_DirEntry entry;
    while(!Filesystem_ReadDir(id, &entry))
        {
            //COM_WriteStr("%s\r\n", entry.dir_name);
        }

        while(1);
    sys_tss.esp0 = kmalloc(KB(16));
    asm volatile(
        "cli \n\t"
        "mov $0x23, %ax\n\t"
        "mov %ax, %ds\n\t"
        "mov %ax, %es\n\t"
        "mov %ax, %fs\n\t"
        "mov %ax, %gs\n\t"
        "mov %esp, %eax\n\t"
        "pushl $0x23\n\t"
        "pushl %eax\n\t"
        "pushf\n\t"
        "pop %eax\n\t"
        "or $512, %eax\n\t"
        "push %eax\n\t"
        "pushl $0x1B\n\t"
        "push $t_main_user\n\t"
        "iret\n\t"
        "t_main_user: \n\t"
    );

    //asm volatile("cli");

    while(1)
        {
            //temp++;
            //temp2 = 0xDEADBEEF;
            asm volatile("mov $0xDEADBEEF, %eax");
        }
}

//extern "C" /* Use C linkage for kernel_main. */
void kernel_main(int argc, char** isKernelMode)
{
    while(1)
        {
            graphics_Clear();
            graphics_DrawBuffer(tmp, 0, 0, 1920, 1080);

            RTC_Time t;
            CMOS_GetRTCTime(&t);

            graphics_WriteUInt64(temp, 2, 0, 16);
            graphics_WriteUInt32(temp2, 16, 0, 32);
            graphics_WriteUInt32(t.seconds, 10, 0, 48);
            graphics_WriteUInt32(sizeof(HBA_FIS), 16, 0, 64);

            graphics_SwapBuffer();
        }
}
