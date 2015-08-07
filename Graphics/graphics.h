#ifndef _GRAPHICS_MAIN_H_
#define _GRAPHICS_MAIN_H_

#include <stddef.h>
#include <stdint.h>
#include "multiboot.h"

#define TILE_X_COUNT 10
#define TILE_Y_COUNT 20
#define TILE_LAYER_COUNT 4

void Graphics_Initialize();
void Graphics_SwapBuffer();
void Graphics_Clear();
void Graphics_SetPixel(uint32_t x, uint32_t y, uint32_t val);
void Graphics_WriteUInt32(uint32_t val, int base, int yOff, int xOff);
void Graphics_WriteUInt64(uint64_t val, int base, int yOff, int xOff);
void Graphics_WriteStr(const char *str, int yOff, int xOff);
void Graphics_WriteFloat(float val, uint32_t decimalCount, int xOff, int yOff);

#endif /* end of include guard: _GRAPHICS_MAIN_H_ */
