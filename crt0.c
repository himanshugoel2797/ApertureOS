/*
 * File:   main.cpp
 * Author: himanshu
 *
 * Created on January 9, 2014, 7:52 PM
 */

#include <stddef.h>
#include <stdint.h>

#include "multiboot.h"

#include "gdt.h"
#include "idt.h"
#include "pic.h"

#include "test.h"
#include "Graphics/font.h"

void writeStr(uint32_t *term, const char *str, int yOff, int xOff, int pitch)
{
        for(int i = 0; str[i] != 0; i++)
        {
                for(int b = 0; b < 8; b++)
                        for(int a = xOff; a < xOff+13; a++) {
                                term[ (yOff+ (8-b) + (a * pitch/4)) ] = ((letters[str[i] - 32][13 - (a - xOff)] >> b) & 1) * -1;
                                //term[ (a + b * pitch) ] = (letters[opts[i]][a] >> b) * -1;
                                //term[ (a + b * pitch) ] = (letters[opts[i]][a] >> b) * -1;
                        }

                yOff+=16;
        }
}

void writeInt(uint32_t *term, uint32_t val, int base, int yOff, int xOff, int pitch)
{
        char str[50];
        char *opts = "0123456789ABCDEF";
        if(base == 16) {
                for(int i = 0; i < 8; i++)
                {
                        str[7 - i] = opts[((val >> (i*4))&0x0F)];
                }
                str[8] = 0;
        }else if(base == 2)
        {
                for(int i = 0; i < 32; i++)
                {
                        str[31 - i] = opts[(val >> i) & 1];
                }
                str[32] = 0;
        }else{
                return;
        }
        writeStr(term, str, yOff, xOff, pitch);
}

int temp = 0;
uint32_t *term_;

//extern "C"{
void setup_kernel_core(multiboot_info_t* mbd, uint32_t magic) {

        GDT_Initialize();
        IDT_Initialize();
        PIC_Initialize();
        uint32_t *term = 0xA0000;

        term = mbd->vbe_mode_info->physbase;
        term_ = term;

        /*for(int b = 0; b < mbd->vbe_mode_info->pitch * mbd->vbe_mode_info->Yres; b +=4)
           {
                term[b] = -1; //B
                term[b+1] = b; //G
                term[b+2] = 0; //R
                term[b+3] = -1; //A
           }*/

        int pitch = mbd->vbe_mode_info->pitch;

        char pixel[4], pixel2[4];

        int yOff = 0, xOff = 0;


        for(int y = yOff; y < height + yOff; y++)
                for(int x = xOff; x < width + xOff; x++)
                {
                        {
                                HEADER_PIXEL(header_data, pixel);
                                pixel2[0] = pixel[2];
                                pixel2[1] = pixel[1];
                                pixel2[2] = pixel[0];
                                term[x + (y * pitch/4)] = *(int*)pixel2;
                        }
                }

        magic = mbd->vbe_control_info->VbeVersion;
        writeStr(term, "VBE version:", 50, 30, pitch);
        writeInt(term, magic, 16, 50, 50, pitch);


        writeStr(term, "Total Memory:", 50, 80, pitch);
        writeInt(term, mbd->vbe_control_info->TotalMemory, 16, 50, 100, pitch);

        asm volatile ("int $0x3");
        asm volatile ("int $0x4");

        writeStr(term, "Off screen memory:", 400, 80, pitch);
        writeInt(term, mbd->vbe_mode_info->offScrSize, 16, 400, 100, pitch);

        writeStr(term, "Attributes:", 400, 160, pitch);
        writeInt(term, mbd->vbe_mode_info->attributes, 2, 400, 200, pitch);

        writeStr(term, "WinA Attributes:", 400, 260, pitch);
        writeInt(term, mbd->vbe_mode_info->winA, 2, 400, 300, pitch);

        writeStr(term, "WinB Attributes:", 400, 360, pitch);
        writeInt(term, mbd->vbe_mode_info->winB, 2, 400, 400, pitch);

        temp = pitch;

}



//extern "C"
void InterruptsTest(int32_t num)
{
        temp = 1;
}

//extern "C" /* Use C linkage for kernel_main. */
void kernel_main() {
    writeInt(term_, &kernel_main, 16, 600,600, temp);
}
