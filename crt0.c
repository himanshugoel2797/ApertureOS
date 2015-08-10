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

#include "memorymanager/bootstrap_mem_manager.h"
#include "memorymanager/memorymanager.h"
#include "memorymanager/pagetable.h"

#include "gdt.h"
#include "idt.h"

#include "drivers/drivers.h"

#include "interruptmanager.h"

#include "utils/native.h"
#include "utils/common.h"

#include "test.h"
#include "Graphics/graphics.h"


int temp = 0, temp2 = 0, y, yOff, x, xOff, rval = 0;
char *tmp;


float r = 3.3f;

uint64_t allocLoc;
size_t q = 0;


void timerHandler(Registers *regs);

void keyboard_test(Registers *regs)
{
        //COM_WriteStr("Keyboard Input Recieved!\r\n");
        temp2 = inb(0x60);
        //outb(0x60, inb(0x61));
        //temp2 = -1;
}

void timerHandler(Registers *regs)
{
        Graphics_Clear();
        COM_WriteStr("Update!\r\n");

        q = 0;
        for(y = 0; y < 1080; y++)
                for(x = 0; x < 1920; x++)
                {
                        Graphics_SetPixel(x,y, *(int*)&tmp[q]);
                        q+=4;
                }
        RTC_Time t;
        CMOS_GetRTCTime(&t);

        Graphics_WriteUInt64(allocLoc, 16, 0, 0);
        Graphics_WriteUInt32(rval, 16, 0, 16);
        Graphics_WriteUInt32(t.seconds, 10, 0, 32);

        Graphics_SwapBuffer();

}

//extern "C"{
void setup_kernel_core(multiboot_info_t* mbd, uint32_t magic) {

        COM_Initialize();

        ACPITables_Initialize();

        GDT_Initialize();
        IDT_Initialize();
        InterruptManager_Initialize();

        CMOS_Initialize();
        APIC_Initialize();
        rval = HPET_Initialize();

        InterruptManager_RegisterHandler(1, 30, timerHandler);
        FPU_Initialize();

        //Backup all important information from the bootloader
        global_vbe_info = Bootstrap_malloc(sizeof(VbeInfoBlock));
        memcpy(global_vbe_info, mbd->vbe_control_info, sizeof(VbeInfoBlock));

        global_mode_info = Bootstrap_malloc(sizeof(ModeInfoBlock));
        memcpy(global_mode_info, mbd->vbe_mode_info, sizeof(ModeInfoBlock));

        global_multiboot_info = Bootstrap_malloc(sizeof(multiboot_info_t));
        memcpy(global_multiboot_info, mbd, sizeof(multiboot_info_t));

        global_memory_map_size = mbd->mmap_length;
        global_memory_map = Bootstrap_malloc(global_memory_map_size);
        memcpy(global_memory_map, mbd->mmap_addr, global_memory_map_size);


        asm ("wbinvd"); //Flush the caches so the dynamic code takes effect

        MemMan_Initialize();
        Paging_Initialize();

        Graphics_Initialize();


        allocLoc = MemMan_Alloc( (31 % 31) * KB(4));
        MemMan_Free(allocLoc, (31 % 31) * KB(4));


        tmp = Bootstrap_malloc(1080*1920*4);
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


        COM_WriteStr("Init!");
        asm ("sti");
        //asm ("int $0x31");
        //asm ("int $0x31");
        InterruptManager_RegisterHandler(32, 0, keyboard_test);
        temp2 = PS2_Initialize();

        while(1) {
                asm ("int $0x1");
        }
        while(1) ;
        asm ("hlt");

}

//extern "C" /* Use C linkage for kernel_main. */
void kernel_main() {
}
