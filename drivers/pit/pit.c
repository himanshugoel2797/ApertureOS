#include "pit.h"
#include "utils/native.h"


uint32_t curFrequency = 0;
void PIT_SetFrequency(uint8_t channel, uint8_t access, uint8_t mode, uint8_t valType, uint32_t frequency)
{
        uint32_t divisor = 1193180 / frequency;
        curFrequency = divisor * 1193180;

        channel &= 3;
        access &= 3;
        mode &= 7;
        valType &= 1;

        // Send the command byte.
        uint8_t cmd = 0;
        cmd = (channel << 6) | (access << 4) | (mode << 1) | valType;

        outb(PIT_CMD, cmd);

        // Divisor has to be sent byte-wise, so split here into upper/lower bytes.
        if((access & PIT_ACCESS_LO_BYTE) == PIT_ACCESS_LO_BYTE)
        {
                uint8_t l = (uint8_t)(divisor & 0xFF);
                outb(0x40 + channel, l);
        }

        if((access & PIT_ACCESS_HI_BYTE) == PIT_ACCESS_HI_BYTE) {
                uint8_t h = (uint8_t)( (divisor>>8) & 0xFF );
                outb(0x40 + channel, h);
        }
}

void PIT_Sleep(uint32_t interval)
{

}
