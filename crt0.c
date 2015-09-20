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
#include "processors.h"

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

    Keyboard_Setup();
    SyscallManager_Initialize();
    Interrupts_Unlock();

    while(1);
}

//extern "C" /* Use C linkage for kernel_main. */
void kernel_main(int argc, char** isKernelMode)
{
    Filesystem_Setup();
    ProcessManager_Initialize();
    Terminal_Start();
    while(1);
}
