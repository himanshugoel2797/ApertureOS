/*
 * File:   main.cpp
 * Author: himanshu
 *
 * Created on January 9, 2014, 7:52 PM
 */

#include <stddef.h>
#include <stdint.h>

#include "multiboot.h"
#include "globals.h"

#include "memorymanager/bootstrap_mem_manager.h"

#include "gdt.h"
#include "idt.h"
#include "pic.h"

#include "fpu.h"
#include "pit.h"

#include "utils/native.h"
#include "utils/common.h"

#include "test.h"
#include "Graphics/graphics.h"


int temp = 0, temp2 = 0, y, yOff, x, xOff;
char pixel[4];

char tmp[1920*1080*4];
float r = 3.3f;

size_t q = 0;

void timerHandler(Registers *regs)
{
        temp2++;

        Graphics_Clear();
        yOff = 0, xOff = 0;

        char *header_data_backup = header_data;

        q = 0;
        for(y = 0; y < 1080; y++)
                for(x = 0; x < 1920; x++)
                {
                        HEADER_PIXEL(header_data, pixel);
                        tmp[q] = pixel[0];
                        tmp[q + 1] = pixel[1];
                        tmp[q + 2] = pixel[2];
                        tmp[q + 3] = pixel[3];
                        Graphics_SetPixel(x,y, *(int*)&tmp[q]);
                        q+=4;
                }
        header_data = header_data_backup;

        Graphics_WriteInt(temp2, 16, 400, 500);

        r = 3.1f * temp2;
        Graphics_WriteFloat(r, 10, 550, 500);
        Graphics_SwapBuffer();
}

//extern "C"{
void setup_kernel_core(multiboot_info_t* mbd, uint32_t magic) {

        GDT_Initialize();
        IDT_Initialize();
        PIC_Initialize();
        FPU_Initialize();


        //global_pm_info = Bootstrap_malloc(mbd->vbe_interface_len);
        //memcpy(global_pm_info, (void*)(mbd->vbe_interface_seg * 0x10 + mbd->vbe_interface_off), mbd->vbe_interface_len);
        global_pm_info = (mbd->vbe_interface_seg * 0x10 + mbd->vbe_interface_off);

        global_vbe_info = Bootstrap_malloc(sizeof(VbeInfoBlock));
        memcpy(global_vbe_info, mbd->vbe_control_info, sizeof(VbeInfoBlock));

        global_mode_info = Bootstrap_malloc(sizeof(ModeInfoBlock));
        memcpy(global_mode_info, mbd->vbe_mode_info, sizeof(ModeInfoBlock));

        global_multiboot_info = Bootstrap_malloc(sizeof(multiboot_info_t));
        memcpy(global_multiboot_info, mbd, sizeof(multiboot_info_t));

        asm ("wbinvd"); //Flush the caches so the dynamic code takes effect

        IDT_RegisterHandler(32, timerHandler);
        Graphics_Initialize();


        uint32_t *term = 0xA0000;

        int pitch = mbd->framebuffer_pitch;
        //term += pitch/4 * mbd->framebuffer_height;

        magic = mbd->vbe_control_info->VbeVersion;
        Graphics_WriteStr("VBE version:", 50, 30);
        Graphics_WriteInt(global_pm_info->setDisplayStart, 16, 50, 50);


        Graphics_WriteStr("Total Memory:", 50, 80);
        Graphics_WriteInt((mbd->vbe_interface_seg * 0x10 + mbd->vbe_interface_off), 16, 50, 100);

        Graphics_WriteStr("Off screen memory:", 400, 80);
        Graphics_WriteInt(mbd->vbe_mode_info->offScrSize, 16, 400, 100);

        Graphics_WriteStr("Attributes:", 400, 160);
        Graphics_WriteInt(mbd->vbe_mode_info->attributes, 2, 400, 200);

        Graphics_WriteStr("WinA Attributes:", 400, 260);
        Graphics_WriteInt(mbd->vbe_mode_info->winA, 2, 400, 300);

        Graphics_WriteStr("WinB Attributes:", 400, 360);
        Graphics_WriteInt(mbd->vbe_mode_info->winB, 2, 400, 400);

//        Graphics_SwapBuffer();



        for(y = 0; y < height; y++)
                for(x = 0; x < width; x++)
                {
                        //HEADER_PIXEL(header_data, pixel);
                        tmp[q] = pixel[0];
                        tmp[q + 1] = pixel[1];
                        tmp[q + 2] = pixel[2];
                        tmp[q + 3] = pixel[3];
                        q+=4;
                        //Graphics_SetPixel(x, y,*(int*)pixel);
                }


        asm ("sti");
        PIT_SetFrequency(PIT_CH0, PIT_ACCESS_LO_BYTE | PIT_ACCESS_HI_BYTE, PIT_MODE_SQUARE_WAVE, PIT_VAL_16BIT, 60);


        /*asm (
                "fldpi\n\t"
                //"fsqrt\n\t"
                "fsts (%0)" : "=r" (r)
                );
         */

        while(1) {
                //temp2++;
        }

}

//extern "C" /* Use C linkage for kernel_main. */
void kernel_main() {
}
