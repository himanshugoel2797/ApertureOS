#ifndef _GRAPHICS_MAIN_H_
#define _GRAPHICS_MAIN_H_

#include "types.h"
#include "multiboot.h"

/**
 * \defgroup graphics Graphics Library
 * @{
 */

#define BLOCK_GROUP_WIDTH 160  //!< The width of the dirty block management table
#define BLOCK_GROUP_HEIGHT 90  //!< The height of the dirty block management table

//! Display related information, can be requested by programs
typedef struct
{
    uint32_t size;                 //!< The size of this structure
    uint32_t width;                //!< The width of the main display
    uint32_t height;               //!< The height of the main display
    uint32_t bpp;                  //!< The bytes per pixel
    uint32_t pitch;
    uint32_t *framebuffer_addr;    //!< The virtual address in kernel memory of the main framebuffer
    uint32_t *backbuffer_addr;     //!< The virtual address in kernel memory of the backbuffer
} DisplayInfo;

//! Initialize the graphics library

//! Note: Requires virtual memory management and FPU
void
graphics_Initialize(void);

//! Retrieve a pointer to the DisplayInfo structure
DisplayInfo*
graphics_GetDisplayInfoPtr(void);

//! Copy the backbuffer contents to the display buffer
void
graphics_SwapBuffer(void);

//! Clear the backbuffer
void
graphics_Clear(void);

//! Set the value of a pixel

//! \param x The X pixel position
//! \param y The Y pixel position
//! \param val The BGRA pixel value
//! \sa graphics_DrawBuffer()
void
graphics_SetPixel(uint32_t x,
                  uint32_t y,
                  uint32_t val);

//! Write a uint32_t to the screen

//! \param val The value to write
//! \param base The base in which the value should be printed
//! \param xOff The X coordinate for the top left corner of the output
//! \param yOff The Y coordinate for the top left corner of the output
//! \sa graphics_WriteUInt64(), graphics_WriteFloat(), graphics_WriteStr()
void
graphics_WriteUInt32(uint32_t val,
                     int base,
                     int xOff,
                     int yOff);

//! Write a uint64_t to the screen

//! \param val The value to write
//! \param base The base in which the value should be printed
//! \param xOff The X coordinate for the top left corner of the output
//! \param yOff The Y coordinate for the top left corner of the output
//! \sa graphics_WriteUInt32(), graphics_WriteFloat(), graphics_WriteStr()
void
graphics_WriteUInt64(uint64_t val,
                     int base,
                     int xOff,
                     int yOff);


//! Write a string to the screen

//! \param str The string to write
//! \param xOff The X coordinate for the top left corner of the output
//! \param yOff The Y coordinate for the top left corner of the output
//! \sa graphics_writeFloat(), graphics_WriteUInt32(), graphics_WriteUInt64()
void
graphics_WriteStr(const char *str,
                  int xOff,
                  int yOff);

//! Write a float to the screen

//! \param val The value to write
//! \param decimalCount The number of decimal places to include
//! \param xOff The X coordinate for the top left corner of the output
//! \param yOff The Y coordinate for the top left corner of the output
//! \sa graphics_writeFloat(), graphics_WriteUInt32(), graphics_WriteUInt64()
void
graphics_WriteFloat(float val,
                    uint32_t decimalCount,
                    int xOff,
                    int yOff);

//! Draw a buffer to the screen

//! \param buffer The RGBA source from which to draw
//! \param x The X coordinate for the top left corner of the output
//! \param y The Y coordinate for the top left corner of the output
//! \param width The width of the image to show
//! \param height The height of the image to show
void
graphics_DrawBuffer(void* buffer,
                    uint32_t x,
                    uint32_t y,
                    uint32_t width,
                    uint32_t height);


void
graphics_Write(const char *str,
               uint32_t x,
               uint32_t y,
               ...);

/**@}*/

#endif /* end of include guard: _GRAPHICS_MAIN_H_ */
