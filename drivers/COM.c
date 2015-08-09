#include "COM.h"

#include "utils/native.h"

#define PORT 0x03F8

void COM_Initialize()
{
  #if COM_ENABLED
        outb(PORT + 1, 0x00); // Disable all interrupts
        outb(PORT + 3, 0x80); // Enable DLAB (set baud rate divisor)
        outb(PORT + 0, 0x03); // Set divisor to 3 (lo byte) 38400 baud
        outb(PORT + 1, 0x00); //                  (hi byte)
        outb(PORT + 3, 0x03); // 8 bits, no parity, one stop bit
        outb(PORT + 2, 0xC7); // Enable FIFO, clear them, with 14-byte threshold
        outb(PORT + 4, 0x0B); // IRQs enabled, RTS/DSR set
        #endif
}

uint8_t COM_ready()
{
  #if COM_ENABLED == 1
        return inb(PORT + 5) & 0x20;
        #else
        return 1;
        #endif

}

void COM_WriteStr(const char *str)
{
  #if COM_ENABLED == 1
        int index = 0;
        while(str[index])
        {
                while(COM_ready() == 0) ;
                outb(PORT, str[index++]);
        }
        #endif
}
