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

char bbuf[1920 * 1080 * 4];

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
  frameBufferB = bootstrap_malloc(buffer_size);

  char *vPointer = virtMemMan_FindEmptyAddress(buffer_size, MEM_KERNEL);
  int retVal = virtMemMan_Map((uint32_t)vPointer, (uint64_t)frameBufferA,
                              buffer_size, MEM_TYPE_WC, MEM_WRITE, MEM_KERNEL);
  frameBufferA = vPointer;

  // frameBufferB = bbuf;
  backBuffer = (uint32_t *)frameBufferB;

  // Initialize both buffers
  memset(frameBufferA, 1, buffer_size);
  memset(frameBufferB, 1, buffer_size);
}

void graphics_SwapBuffer() { memcpy(frameBufferA, frameBufferB, buffer_size); }

void graphics_Clear() { memset(frameBufferB, 1, buffer_size); }

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
