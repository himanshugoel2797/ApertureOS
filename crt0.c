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

#include "test.h"
#include "graphics/graphics.h"


int temp = 0, temp2 = 0, y, yOff, x, xOff, rval = 0;
char *tmp;
uint64_t allocLoc = 0;
size_t q = 0;

void timerHandler()
{
        //asm volatile("cli");
        temp++;
        graphics_Clear();
        graphics_DrawBuffer(tmp, 0, 0, 1920, 1080);

        RTC_Time t;
        CMOS_GetRTCTime(&t);

        graphics_WriteUInt64(temp, 2, 0, 16);
        graphics_WriteUInt32(temp2, 16, 0, 32);
        graphics_WriteUInt32(t.seconds, 10, 0, 48);
        graphics_WriteUInt32(sizeof(Thread), 10, 0, 64);


        graphics_SwapBuffer();
        //asm volatile("sti");
}

//extern "C"{
void setup_kernel_core(multiboot_info_t* mbd, uint32_t magic) {

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

        kmalloc_init();
        Timers_Setup();
        ThreadMan_Setup();

        tmp = kmalloc(1080*1920*4 + 16);
        tmp += 16;
        tmp = ((uint32_t)tmp) & ~0xf;
        char pixel[4];


        for(y = 0; y < height; y++)
                for(x = 0; x < width; x++)
                {
                        HEADER_PIXEL(header_data, pixel);
                        tmp[q] = pixel[2];
                        tmp[q + 1] = pixel[1];
                        tmp[q + 2] = pixel[0];
                        tmp[q + 3] = pixel[3];
                        q+=4;
                }
        Keyboard_Setup();

        Interrupts_Unlock();

        //UID id = Timers_CreateNew(5, TRUE, timerHandler);
        //Timers_StartTimer(id);

        while(1) {
                asm ("hlt");
        }


}

//extern "C" /* Use C linkage for kernel_main. */
void kernel_main(int argc, char** isKernelMode) {
        while(1) {
                timerHandler();
        }
}
