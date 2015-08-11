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

uint64_t allocLoc = 0;
size_t q = 0;


void timerHandler(Registers *regs);

void keyboard_test(Registers *regs)
{
        COM_WriteStr("Keyboard Input Recieved!\r\n");
        temp2 = inb(0x60);
        allocLoc++;
        //outb(0x60, inb(0x61));
        //temp2 = -1;
}

void timerHandler(Registers *regs)
{
        temp++;
        if(temp % 1 == 0) {
                Graphics_Clear();

                q = 0;
                for(y = 0; y < 1080; y++)
                        for(x = 0; x < 1920; x++)
                        {
                                Graphics_SetPixel(x,y, *(int*)&tmp[q]);
                                q+=4;
                        }
                RTC_Time t;
                CMOS_GetRTCTime(&t);

                Graphics_WriteUInt64(temp2, 16, 0, 16);
                Graphics_WriteUInt32(rval, 16, 0, 32);
                Graphics_WriteUInt32(t.seconds, 2, 0, 48);
                Graphics_WriteUInt32(allocLoc, 2, 0, 64);

                Graphics_SwapBuffer();
        }
}

//extern "C"{
void setup_kernel_core(multiboot_info_t* mbd, uint32_t magic) {

        COM_Initialize();

        ACPITables_Initialize();

        GDT_Initialize();
        IDT_Initialize();
        InterruptManager_Initialize();

        InterruptManager_RegisterHandler(31, 0, timerHandler);
        InterruptManager_RegisterHandler(32, 0, keyboard_test);

        CMOS_Initialize();
        APIC_Initialize();
        rval = HPET_Initialize();
        HPET_SetTimerConfig(0, 32, 1, 1, 1, 1000);
        HPET_SetGlobalCounter(0);
        HPET_SetEnable(1);






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


        //asm ("wbinvd"); //Flush the caches so the dynamic code takes effect

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

        //PIT_SetFrequency(PIT_CH0, PIT_ACCESS_LO_BYTE | PIT_ACCESS_HI_BYTE, PIT_MODE_SQUARE_WAVE, PIT_VAL_16BIT, 30000);

        //InterruptManager_RegisterHandler(33, 0, keyboard_test);
        PS2_Initialize();

        asm ("sti");
        uint32_t timer = APIC_Read(APIC_TIMER);
        timer &= ~(3<<17);
        timer |= (1<<17);
        APIC_Write(APIC_TIMER, timer);

        APIC_Write(0x380, 256);

        APIC_SetVector(APIC_TIMER, 31);
        APIC_SetEnableInterrupt(APIC_TIMER, 1);


        while(1) {
        }

        asm ("hlt");

}

//extern "C" /* Use C linkage for kernel_main. */
void kernel_main() {
}
