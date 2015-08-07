#include "graphics.h"

#include "memorymanager/bootstrap_mem_manager.h"
#include "utils/common.h"
#include "globals.h"

#include "font.h"

int tileWidth = 0, tileHeight = 0;

char *frameBufferA, *frameBufferB;
uint32_t *backBuffer;

size_t buffer_size;
uint32_t pitch, width, height;
uint8_t bpp;

char bbuf[1920*1080*4];

void Graphics_Initialize()
{
        pitch = global_multiboot_info->framebuffer_pitch/sizeof(uint32_t);
        width = global_multiboot_info->framebuffer_width;
        height = global_multiboot_info->framebuffer_height;
        bpp = global_multiboot_info->framebuffer_bpp;

        //Determine the graphics resolution if 1920x1080 wasn't available
        tileWidth = width / TILE_X_COUNT;
        tileHeight = height / TILE_Y_COUNT;

        buffer_size = (pitch * height * sizeof(uint32_t));

        //Specify the pointers for both framebuffers
        frameBufferA = global_multiboot_info->framebuffer_addr;
        frameBufferB = Bootstrap_malloc(buffer_size);

        //frameBufferB = bbuf;
        backBuffer = (uint32_t*)frameBufferB;

        //Initialize both buffers
        memset(frameBufferA, 1, buffer_size);
        memset(frameBufferB, 1, buffer_size);
}

void Graphics_SwapBuffer()
{
        memcpy(frameBufferA, frameBufferB, buffer_size);
}

void Graphics_Clear()
{
        memset(frameBufferB, 1, buffer_size);
}

void Graphics_WriteStr(const char *str, int yOff, int xOff)
{
        for(int i = 0; str[i] != 0; i++)
        {
                for(int b = 0; b < 8; b++)
                        for(int a = xOff; a < xOff+13; a++) {
                                backBuffer[ (yOff+ (8-b) + (a * pitch)) ] = ((letters[str[i] - 32][13 - (a - xOff)] >> b) & 1) * -1;
                        }

                yOff+=8;
        }
}

void Graphics_WriteUInt32(uint32_t val, int base, int yOff, int xOff)
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
        }else if(base < 16)
        {
                int pos = 0;

                do {
                        str[pos++] = opts[val % base];
                        val /= base;
                }
                while(val != 0);

                str[pos] = 0;
                strrev(str);
        }else
        {
                return;
        }
        Graphics_WriteStr(str, yOff, xOff);
}

void Graphics_WriteUInt64(uint64_t val, int base, int yOff, int xOff)
{
        char str[50];
        char *opts = "0123456789ABCDEF";
        if(base == 16) {
                for(int i = 0; i < 16; i++)
                {
                        str[15 - i] = opts[((val >> (i*4))&0x0F)];
                }
                str[16] = 0;
        }else if(base == 2)
        {
                for(int i = 0; i < 64; i++)
                {
                        str[63 - i] = opts[(val >> i) & 1];
                }
                str[64] = 0;
        }else if(base <= 16)
        {
                int pos = 0;

                do {
                        str[pos++] = opts[val % base];
                        val /= base;
                }
                while(val != 0);

                str[pos] = 0;
                strrev(str);
        }else
        {
                return;
        }
        Graphics_WriteStr(str, yOff, xOff);
}

void Graphics_WriteFloat(float val, uint32_t decimalCount, int xOff, int yOff)
{
        char str[256];
        char opts[] = "0123456789";

        long long val_L2 = val;
        long long val_L = (val * decimalCount) - val_L2;

        int pos = 0;
        do {
                str[pos++] = opts[val_L2 % 10];
                val_L2 /= 10;
        }
        while(val_L2 != 0);

        str[pos] = 0;
        strrev(str);
        str[pos++] = '.';

        do {
                str[pos++] = opts[val_L % 10];
                val_L /= 10;
        }
        while(val_L != 0);

        str[pos] = 0;
        //strrev(str);
        Graphics_WriteStr(str, xOff, yOff);
}

void Graphics_SetPixel(uint32_t x, uint32_t y, uint32_t val)
{
        backBuffer[x + (y * pitch)] = val;
}
