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

#include "memorymanager/priv_memorymanager.h"
#include "memorymanager/priv_paging.h"

#include "gdt.h"
#include "idt.h"
#include "pic.h"
#include "cmos.h"

#include "fpu.h"
#include "pit.h"

#include "drivers/drivers.h"
#include "drivers/ps2/priv_ps2.h"

#include "interruptmanager.h"

#include "utils/native.h"
#include "utils/common.h"

#include "test.h"
#include "Graphics/graphics.h"


int temp = 0, temp2 = 0, y, yOff, x, xOff;
char *tmp;


float r = 3.3f;

uint64_t allocLoc;
size_t q = 0;


void timerHandler(Registers *regs);

void keyboard_test(Registers *regs)
{
        COM_WriteStr("Keyboard Input Recieved!\r\n");
        temp2 = inb(0x60);
        //outb(0x60, inb(0x61));
        //temp2 = -1;
}

void timerHandler(Registers *regs)
{
        temp++;
        temp = temp % 10;
        if(temp == 0) {
                //temp2++;
                Graphics_Clear();

                COM_WriteStr("Graphics Update!");

                //allocLoc = MemMan_Alloc( (temp2 % 31) * KB(4));
                //MemMan_Free(allocLoc, (temp2 % 31) * KB(4));

                //asm volatile("movl %%cr0, %0" : "=r"(temp2));

                q = 0;
                for(y = 0; y < 1080; y++)
                        for(x = 0; x < 1920; x++)
                        {
                                Graphics_SetPixel(x,y, *(int*)&tmp[q]);
                                q+=4;
                        }
                RTC_Time t;
                CMOS_GetRTCTime(&t);

                Graphics_WriteUInt32(temp2, 16, 0, 0);
                uint32_t *pdu = pd_nopse;
                for(int i = 0; i < 1024; i++) {
                        //Graphics_WriteUInt32(pd_pse[i].low_addr, 2, 0, 20 * (i+1));
                }
                for(int i = 0; i < 40; i++) {
                        //Graphics_WriteUInt32(KB4_Blocks_Bitmap[lastNonFullPage + i], 2, 200, 20 * (i+1));
                }
                Graphics_WriteUInt32(lastNonFullPage, 16, 800, 0);
                Graphics_WriteUInt32(GET_FREE_BITCOUNT(lastNonFullPage), 10, 950, 0);
                Graphics_WriteStr("Last Non Full Page", 1000, 0);

                Graphics_WriteUInt32(lastFourthEmptyPage, 16, 800, 20);
                Graphics_WriteUInt32(GET_FREE_BITCOUNT(lastFourthEmptyPage), 10, 950, 20);
                Graphics_WriteStr("Last Fourth Empty Page", 1000, 20);


                Graphics_WriteUInt32(lastHalfEmptyPage, 16, 800, 40);
                Graphics_WriteUInt32(GET_FREE_BITCOUNT(lastHalfEmptyPage), 10, 950, 40);
                Graphics_WriteStr("Last Half Empty Page", 1000, 40);


                Graphics_WriteUInt32(lastEmptyPage, 16, 800, 60);
                Graphics_WriteUInt32(GET_FREE_BITCOUNT(lastEmptyPage), 10, 950, 60);
                Graphics_WriteStr("Last Empty Page", 1000, 60);


                Graphics_WriteUInt32(lastFourthFullPage, 16, 800, 80);
                Graphics_WriteUInt32(GET_FREE_BITCOUNT(lastFourthFullPage), 10, 950, 80);
                Graphics_WriteStr("Last Fourth Full Page", 1000, 80);

                Graphics_SwapBuffer();
        }
}

//extern "C"{
void setup_kernel_core(multiboot_info_t* mbd, uint32_t magic) {

        COM_Initialize();

        GDT_Initialize();
        IDT_Initialize();
        PIC_Initialize();
        InterruptManager_Initialize();

        InterruptManager_RegisterHandler(32, 30, timerHandler);
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
        PIT_SetFrequency(PIT_CH0, PIT_ACCESS_LO_BYTE | PIT_ACCESS_HI_BYTE, PIT_MODE_SQUARE_WAVE, PIT_VAL_16BIT, 75);


        asm ("sti");
        InterruptManager_RegisterHandler(32, 0, keyboard_test);
        temp2 = PS2_Initialize();
        COM_WriteStr("const char *str");

        while(1) {temp2++; }

        asm ("hlt");

}

//extern "C" /* Use C linkage for kernel_main. */
void kernel_main() {
}
