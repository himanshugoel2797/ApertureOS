#include "graphics.h"

#include "managers.h"

#include "utils/common.h"
#include "globals.h"

#include "font.h"

int tileWidth = 0, tileHeight = 0;

char *frameBufferA, *frameBufferB;
uint32_t *backBuffer;

size_t buffer_size;
uint32_t pitch, width, height;
uint8_t bpp;

char tmpBuf[16] __attribute__((aligned(16)));

void graphics_Initialize() {
        pitch = global_multiboot_info->framebuffer_pitch / sizeof(uint32_t);
        width = global_multiboot_info->framebuffer_width;
        height = global_multiboot_info->framebuffer_height;
        bpp = global_multiboot_info->framebuffer_bpp;

        // Determine the graphics resolution if 1920x1080 wasn't available
        tileWidth = width / TILE_X_COUNT;
        tileHeight = height / TILE_Y_COUNT;

        buffer_size = (pitch * height * sizeof(uint32_t));

        // Specify the pointers for both framebuffers
        frameBufferA = (char *)global_multiboot_info->framebuffer_addr;
        frameBufferB = bootstrap_malloc(buffer_size + 16 + 0x70);
        frameBufferB = frameBufferB + 16;
        frameBufferB = ((uint32_t)frameBufferB) & ~0xf;

        char *vPointer = virtMemMan_FindEmptyAddress(buffer_size, MEM_KERNEL);
        int retVal = virtMemMan_Map((uint32_t)vPointer, (uint64_t)frameBufferA,
                                    buffer_size + 0x70, MEM_TYPE_WC, MEM_WRITE, MEM_KERNEL);
        frameBufferA = vPointer;

        backBuffer = (uint32_t *)frameBufferB;

        // Initialize both buffers
        memset(frameBufferA, 0, buffer_size);
        memset(frameBufferB, 0, buffer_size);
}

void graphics_SwapBuffer() {
        uint64_t *fbufA = (uint64_t*)frameBufferA, *fbufB = (uint64_t*)frameBufferB;
        for(uint32_t a = 0; a < buffer_size; a+=0x80)
        {
                asm volatile ("movaps (%%ebx), %%xmm1\n\t"
                              "movaps +0x10(%%ebx), %%xmm2\n\t"
                              "movaps +0x20(%%ebx), %%xmm3\n\t"
                              "movaps +0x30(%%ebx), %%xmm4\n\t"
                              "movaps +0x40(%%ebx), %%xmm5\n\t"
                              "movaps +0x50(%%ebx), %%xmm6\n\t"
                              "movaps +0x70(%%ebx), %%xmm0\n\t"
                              "movaps +0x60(%%ebx), %%xmm7\n\t"
                              "movaps %%xmm0, +0x70(%%eax)\n\t"
                              "movaps %%xmm1, (%%eax)\n\t"
                              "movaps %%xmm2, +0x10(%%eax)\n\t"
                              "movaps %%xmm3, +0x20(%%eax)\n\t"
                              "movaps %%xmm4, +0x30(%%eax)\n\t"
                              "movaps %%xmm5, +0x40(%%eax)\n\t"
                              "movaps %%xmm6, +0x50(%%eax)\n\t"
                              "movaps %%xmm7, +0x60(%%eax)\n\t"
                              : "=a" (fbufA) : "b" (fbufB) : "%xmm0","%xmm1","%xmm2","%xmm3","%xmm4","%xmm5","%xmm6","%xmm7", "%eax","%ebx");

                fbufB+=0x80/sizeof(uint64_t);
                fbufA+=0x80/sizeof(uint64_t);
        }

}

void graphics_Clear()
{
        uint64_t *bbuffer = (uint64_t*)frameBufferB;
        memset(tmpBuf, 0xff, 16);
        asm volatile ("movaps (%0), %%xmm1" :: "r" (tmpBuf) : "%xmm1");
        for(uint32_t a = 0; a < buffer_size; a+=16)
        {
                asm volatile ("movaps %%xmm1, (%0)" : "=r" (bbuffer));
                bbuffer+=2;
        }
}

void graphics_WriteStr(const char *str, int yOff, int xOff) {
        uint32_t curBufVal = 0;

        for (int i = 0; str[i] != 0; i++) {
                for (int b = 0; b < 8; b++)
                        for (int a = xOff; a < xOff + 13; a++) {

                                curBufVal = backBuffer[(yOff + (8 - b) + (a * pitch))];

                                backBuffer[(yOff + (8 - b) + (a * pitch))] =
                                        (1 - ((letters[str[i] - 32][13 - (a - xOff)] >> b) & 1)) *
                                        curBufVal;

                                // if(backBuffer[ (yOff+ (8-b) + (a * pitch)) ] == 0)backBuffer =
                                // curBufVal;
                        }

                yOff += 8;
        }
}

void graphics_WriteUInt32(uint32_t val, int base, int yOff, int xOff) {
        char str[128];
        if (base == 10)
                sprintf(str, "%u", val);
        else if (base == 16)
                sprintf(str, "%#x", val);
        else if (base == 2)
                sprintf(str, "%bb", val);
        graphics_WriteStr(str, yOff, xOff);
}

void graphics_WriteUInt64(uint64_t val, int base, int yOff, int xOff) {
        char str[512];
        if (base == 10)
                sprintf(str, "%u", val);
        else if (base == 16)
                sprintf(str, "%#x", val);
        else if (base == 2)
                sprintf(str, "%bb", val);
        graphics_WriteStr(str, yOff, xOff);
}

void graphics_WriteFloat(float val, uint32_t decimalCount, int xOff, int yOff) {
        char str[256];
        char opts[] = "0123456789";

        long long val_L2 = val;
        long long val_L = (val * decimalCount) - val_L2;

        int pos = 0;
        do {
                str[pos++] = opts[val_L2 % 10];
                val_L2 /= 10;
        } while (val_L2 != 0);

        str[pos] = 0;
        strrev(str);
        str[pos++] = '.';

        do {
                str[pos++] = opts[val_L % 10];
                val_L /= 10;
        } while (val_L != 0);

        str[pos] = 0;
        // strrev(str);
        graphics_WriteStr(str, xOff, yOff);
}

void graphics_SetPixel(uint32_t x, uint32_t y, uint32_t val) {
        backBuffer[x + (y * pitch)] = val;
}

void graphics_DrawBuffer(void* buffer, uint32_t x, uint32_t y, uint32_t w, uint32_t h)
{
        uint64_t *offset = (uint64_t*)&backBuffer[x+(y*pitch)];
        uint64_t* src = (uint64_t*)buffer;

        uint64_t x0 = 0, y0= 0;

        if(x+w > width) w = width - x;
        if(y+h > height) h = height - y;

        while(y0 < h)
        {
                offset[0] = src[0];
                offset[1] = src[1];
                //asm volatile ("movaps (%%ebx), %%xmm1\n\t"
                //              "movaps %%xmm1, (%%eax)" : "=a" (offset): "b" (src) : "%xmm1", "%eax","%ebx" );

                offset+=2;
                x0+=4;
                src+=2;
                if(x0 >= w) {
                        x0 = 0;
                        y0++;
                        offset = (uint64_t*)&backBuffer[x + ((y+y0)*pitch)];
                }

        }
}
