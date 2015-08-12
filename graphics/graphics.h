#ifndef _GRAPHICS_MAIN_H_
#define _GRAPHICS_MAIN_H_

#include "types.h"
#include "multiboot.h"

#define TILE_X_COUNT 10
#define TILE_Y_COUNT 20
#define TILE_LAYER_COUNT 4

void graphics_Initialize();
void graphics_SwapBuffer();
void graphics_Clear();
void graphics_SetPixel(uint32_t x, uint32_t y, uint32_t val);
void graphics_WriteUInt32(uint32_t val, int base, int yOff, int xOff);
void graphics_WriteUInt64(uint64_t val, int base, int yOff, int xOff);
void graphics_WriteStr(const char *str, int yOff, int xOff);
void graphics_WriteFloat(float val, uint32_t decimalCount, int xOff, int yOff);

#endif /* end of include guard: _GRAPHICS_MAIN_H_ */
