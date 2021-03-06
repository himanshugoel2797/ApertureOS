#include "graphics.h"

#include "managers.h"

#include "utils/common.h"
#include "globals.h"

#include "font.h"

char *frameBufferA, *frameBufferB;
static uint32_t *backBuffer;
static bool dirty_table[BLOCK_GROUP_WIDTH][BLOCK_GROUP_HEIGHT];
static int block_index_div;
static DisplayInfo disp_info;

static size_t buffer_size;
static uint32_t pitch, width, height;
static uint8_t bpp;

char tmpBuf[16] __attribute__((aligned(16)));

void
graphics_Initialize(void)
{
    memset(dirty_table, 0, BLOCK_GROUP_WIDTH * BLOCK_GROUP_HEIGHT);

    pitch = global_multiboot_info->framebuffer_pitch / sizeof(uint32_t);
    width = global_multiboot_info->framebuffer_width;
    height = global_multiboot_info->framebuffer_height;
    bpp = global_multiboot_info->framebuffer_bpp;

    block_index_div = width/BLOCK_GROUP_WIDTH;
    // Determine the graphics resolution if 1920x1080 wasn't available

    buffer_size = (pitch * height * sizeof(uint32_t));

    // Specify the pointers for both framebuffers
    frameBufferA = (char*)global_multiboot_info->framebuffer_addr;
    frameBufferB = bootstrap_malloc(buffer_size + 0x80);
    frameBufferB = frameBufferB + 0x80;
    frameBufferB -= ((uint32_t)frameBufferB) % 0x80;

    char *vPointer = virtMemMan_FindEmptyAddress (buffer_size,
                     MEM_KERNEL);

    int retVal = virtMemMan_Map ((uint32_t)vPointer,
                                 (uint64_t)frameBufferA,
                                 buffer_size,
                                 MEM_TYPE_WC,
                                 MEM_WRITE,
                                 MEM_KERNEL);

    COM_WriteStr("Graphics Pointer: %x\r\n", frameBufferB);
    frameBufferA = vPointer;

    backBuffer = (uint32_t *)frameBufferB;

    // Initialize both buffers
    memset (frameBufferA, 0, buffer_size);
    memset (frameBufferB, 0, buffer_size);

    disp_info.size = sizeof(DisplayInfo);
    disp_info.width = global_multiboot_info->framebuffer_width;
    disp_info.height = global_multiboot_info->framebuffer_height;
    disp_info.bpp = global_multiboot_info->framebuffer_bpp;
    disp_info.pitch = global_multiboot_info->framebuffer_pitch;
    disp_info.framebuffer_addr = frameBufferA;
    disp_info.backbuffer_addr = frameBufferB;

}

DisplayInfo*
graphics_GetDisplayInfoPtr(void)
{
    return &disp_info;
}

void
graphics_SwapBuffer(void)
{
    uint64_t *fbufA = (uint64_t*)frameBufferA, *fbufB = (uint64_t*)frameBufferB;

    int x = 0, y = 0;

    for (uint32_t a = 0; a < buffer_size; a+=0x80)
        {
            //if(dirty_table[x / block_index_div][y / block_index_div])
            {
                asm volatile ("movdqa (%%ebx), %%xmm0\n\t"
                              "movdqa +0x10(%%ebx), %%xmm1\n\t"
                              "movdqa +0x20(%%ebx), %%xmm2\n\t"
                              "movdqa +0x30(%%ebx), %%xmm3\n\t"
                              "movdqa +0x40(%%ebx), %%xmm4\n\t"
                              "movdqa +0x50(%%ebx), %%xmm5\n\t"
                              "movdqa +0x60(%%ebx), %%xmm6\n\t"
                              "movdqa +0x70(%%ebx), %%xmm7\n\t"
                              "shufps $0xE4, %%xmm0,  %%xmm0\n\t"
                              "shufps $0xE4, %%xmm1,  %%xmm1\n\t"
                              "shufps $0xE4, %%xmm2,  %%xmm2\n\t"
                              "shufps $0xE4, %%xmm3,  %%xmm3\n\t"
                              "shufps $0xE4, %%xmm4,  %%xmm4\n\t"
                              "shufps $0xE4, %%xmm5,  %%xmm5\n\t"
                              "shufps $0xE4, %%xmm6,  %%xmm6\n\t"
                              "shufps $0xE4, %%xmm7,  %%xmm7\n\t"
                              "movntdq %%xmm0, (%%eax)\n\t"
                              "movntdq %%xmm1, +0x10(%%eax)\n\t"
                              "movntdq %%xmm2, +0x20(%%eax)\n\t"
                              "movntdq %%xmm3, +0x30(%%eax)\n\t"
                              "movntdq %%xmm4, +0x40(%%eax)\n\t"
                              "movntdq %%xmm5, +0x50(%%eax)\n\t"
                              "movntdq %%xmm6, +0x60(%%eax)\n\t"
                              "movntdq %%xmm7, +0x70(%%eax)\n\t"
                              :: "a" (fbufA), "b" (fbufB) : "%xmm0","%xmm1","%xmm2","%xmm3","%xmm4","%xmm5","%xmm6","%xmm7", "%eax","%ebx");
            }

            fbufB+=0x80/sizeof(uint64_t);
            fbufA+=0x80/sizeof(uint64_t);
            x += 0x80/sizeof(uint32_t);
            if(x >= width)
                {
                    x = x % width;
                    y++;
                }
        }

    memset (dirty_table, 0, BLOCK_GROUP_WIDTH * BLOCK_GROUP_HEIGHT);
}

void
graphics_Clear(void)
{
    uint64_t *bbuffer = (uint64_t*)frameBufferB;
    memset (tmpBuf, 0x00, 16);
    memset (dirty_table, 1, BLOCK_GROUP_WIDTH * BLOCK_GROUP_HEIGHT);

    for (uint32_t a = 0; a < buffer_size; a+=0x80)
        {
            asm volatile ("movdqa (%0), %%xmm0" :: "a" (tmpBuf));
            asm volatile ("movdqa (%0), %%xmm1" :: "a" (tmpBuf));
            asm volatile ("movdqa (%0), %%xmm2" :: "a" (tmpBuf));
            asm volatile ("movdqa (%0), %%xmm3" :: "a" (tmpBuf));
            asm volatile ("movdqa (%0), %%xmm4" :: "a" (tmpBuf));
            asm volatile ("movdqa (%0), %%xmm5" :: "a" (tmpBuf));
            asm volatile ("movdqa (%0), %%xmm6" :: "a" (tmpBuf));
            asm volatile ("movdqa (%0), %%xmm7" :: "a" (tmpBuf));
            asm volatile ("movntdq %%xmm0, (%0)":: "b" (bbuffer));
            asm volatile ("movntdq %%xmm1, +0x10(%0)":: "b" (bbuffer));
            asm volatile ("movntdq %%xmm2, +0x20(%0)":: "b" (bbuffer));
            asm volatile ("movntdq %%xmm3, +0x30(%0)":: "b" (bbuffer));
            asm volatile ("movntdq %%xmm4, +0x40(%0)":: "b" (bbuffer));
            asm volatile ("movntdq %%xmm5, +0x50(%0)":: "b" (bbuffer));
            asm volatile ("movntdq %%xmm6, +0x60(%0)":: "b" (bbuffer));
            asm volatile ("movntdq %%xmm7, +0x70(%0)":: "b" (bbuffer));
            bbuffer+=0x80/sizeof(uint64_t);
        }
}

void
graphics_WriteStr(const char *str,
                  int yOff,
                  int xOff)
{
    if(xOff + 13 > height)return;
    if(yOff + 8 > width)return;

    uint32_t curBufVal = 0;

    for (int i = 0; str[i] != 0; i++)
        {
            for (int b = 0; b < 8; b++)
                {
                    for (int a = xOff; a < xOff + 13; a++)
                        {

                            //Mark the relevant blocks as dirty
                            int dirty_block_x = (yOff + (8 - b))/block_index_div;
                            int dirty_block_y = (xOff + (13 - a))/block_index_div;
                            dirty_table[dirty_block_x][dirty_block_y] = TRUE;

                            curBufVal = backBuffer[(yOff + (8 - b) + (a * pitch))];

                            backBuffer[(yOff + (8 - b) + (a * pitch))] =
                                (((letters[str[i] - 32][13 - (a - xOff)] >> b) & 1)) *
                                (0xDDDDDDDD);

                            // if(backBuffer[ (yOff+ (8-b) + (a * pitch)) ] == 0)backBuffer =
                            // curBufVal;
                        }
                }
            yOff += 8;
        }
}

void
graphics_WriteUInt32(uint32_t val,
                     int base,
                     int xOff,
                     int yOff)
{
    char str[128];
    if (base == 10)
        sprintf(str, "%u", val);
    else if (base == 16)
        sprintf(str, "%#x", val);
    else if (base == 2)
        sprintf(str, "%bb", val);
    graphics_WriteStr(str, xOff, yOff);
}

void
graphics_WriteUInt64(uint64_t val,
                     int base,
                     int xOff,
                     int yOff)
{
    char str[512];
    if (base == 10)
        sprintf(str, "%u", val);
    else if (base == 16)
        sprintf(str, "%#x", val);
    else if (base == 2)
        sprintf(str, "%bb", val);
    graphics_WriteStr(str, xOff, yOff);
}

void
graphics_WriteFloat(float val,
                    uint32_t decimalCount,
                    int xOff,
                    int yOff)
{
    char str[256];
    char opts[] = "0123456789";

    long long val_L2 = val;
    long long val_L = (val * decimalCount) - val_L2;

    int pos = 0;
    do
        {
            str[pos++] = opts[val_L2 % 10];
            val_L2 /= 10;
        }
    while (val_L2 != 0);

    str[pos] = 0;
    strrev(str);
    str[pos++] = '.';

    do
        {
            str[pos++] = opts[val_L % 10];
            val_L /= 10;
        }
    while (val_L != 0);

    str[pos] = 0;
    // strrev(str);
    graphics_WriteStr(str, xOff, yOff);
}

void
graphics_SetPixel(uint32_t x,
                  uint32_t y,
                  uint32_t val)
{
    //Mark the relevant blocks as dirty
    int dirty_block_x = x/block_index_div;
    int dirty_block_y = y/block_index_div;
    dirty_table[dirty_block_x][dirty_block_y] = TRUE;
    backBuffer[x + (y * pitch)] = val;
}

void
graphics_DrawBuffer(void* buffer,
                    uint32_t x,
                    uint32_t y,
                    uint32_t w,
                    uint32_t h)
{
    uint8_t* offset = (uint8_t*)&backBuffer[x+(y*pitch)];
    uint8_t* src = (uint8_t*)buffer;

    uint64_t x0 = 0, y0= 0;
    uint64_t tmp0 = 0, tmp1 = 0;

    if (x > width) x = 0;
    if (y > height) y = 0;

    x0 = x;
    y0 = y;

    if (x+w > width) w = width - x;
    if (y+h > height) h = height - y;

    while (y0 < h)
        {
            offset[0] = src[2];
            offset[1] = src[1];
            offset[2] = src[0];
            offset[3] = src[3];

            //asm volatile ("movaps (%%ebx), %%xmm0\n\t"
            //                      "shufps $0xE4, %%xmm0,  %%xmm0\n\t"
            //              "movaps %%xmm0, (%%eax)" :: "a" (offset), "b" (src));


            //Mark the relevant blocks as dirty
            int dirty_block_x = (x+x0)/block_index_div;
            int dirty_block_y = (y+y0)/block_index_div;
            dirty_table[dirty_block_x][dirty_block_y] = TRUE;

            offset+=1 * 4;
            x0+=1;
            src+=1 * 4;
            if(x0 >= w)
                {
                    x0 = 0;
                    y0++;
                    offset = (uint64_t*)&backBuffer[x + ((y+y0)*pitch)];
                }

        }
}

void
graphics_Write(const char *fmt,
               uint32_t x,
               uint32_t y,
               ...)
{

    char str[1024];
    memset(str, 0, 1024);
    int index = 0;
    va_list vl;
    va_start(vl, fmt);
    vsnprintf(str, fmt, vl);
    va_end(vl);
    graphics_WriteStr(str, x, y);
}