#include "mouse.h"
#include "utils/common.h"

static int cursorX, cursorY;
static bool mouse_btns[20];
static int scroll_wheelX, scroll_wheelY;

static void
Mouse_IntHandler(Registers *regs);

void
Mouse_Initialize(void)
{
    Interrupts_RegisterHandler(IRQ(12), 0, Mouse_IntHandler);
    IOAPIC_SetEnableMode(IRQ(12), ENABLE);

    cursorX = 0;
    cursorY = 0;
    scroll_wheelX = 0;
    scroll_wheelY = 0;

    memset(mouse_btns, 0, 0x20 * sizeof(bool));
}

static void
Mouse_IntHandler(Registers *regs)
{
    if(inb(0x64) & (0x21))
        {
            uint8_t b0 = inb(0x60);
            int delta_x = (int)inb(0x60);
            int delta_y = (int)inb(0x60);

            uint8_t extra = 0;
            if(PS2Mouse_HasScrollWheel() | PS2Mouse_IsFiveButton())extra = inb(0x60);

            if((b0 >> 6) == 0)
                {
                    mouse_btns[0] = (b0 >> 1) & 1;
                    mouse_btns[1] = (b0 & 1);
                    mouse_btns[2] = (b0 >> 2) & 1;

                    if((b0 >> 4) & 1)delta_x |= 0xFFFFFF00;
                    if((b0 >> 5) & 1)delta_y |= 0xFFFFFF00;

                    if(PS2Mouse_HasScrollWheel())
                        {
                            switch(extra & 0x0F)
                                {
                                case 1:
                                    scroll_wheelY++;
                                    break;
                                case 2:
                                    scroll_wheelX++;
                                    break;
                                case 0xE:
                                    scroll_wheelX--;
                                    break;
                                case 0xF:
                                    scroll_wheelY--;
                                    break;
                                }
                        }

                    if(PS2Mouse_IsFiveButton())
                        {
                            mouse_btns[3] = (extra >> 4) & 1;
                            mouse_btns[4] = (extra >> 5) & 1;
                        }
                }
        }
}