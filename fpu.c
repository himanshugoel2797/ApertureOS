#include "fpu.h"

void FPU_Initialize()
{
        uint32_t bitmask = 0;
        //Get the current control register 0 value
        asm volatile ("mov %%cr0, %0" : "=r" (bitmask));

        bitmask &= ~(1 << 2); //Enable the FPU
        bitmask |= (1 << 5); //Enable FPU exceptions
        //bitmask |= (1 << 3); //Allow CPU to backup FPU state
        //bitmask |= (1 << 1); //FWAIT should cause a state change

        //Set the control register to the new value
        asm volatile ("mov %0, %%cr0" :: "r" (bitmask));

        //Get the current control register 0 value
        asm volatile ("mov %%cr4, %0" : "=r" (bitmask));

        bitmask |= (1 << 9); //Enable FPU exceptions
        bitmask |= (1 << 10); //Allow CPU to backup FPU state
        bitmask |= (1 << 1); //FWAIT should cause a state change

        //Set the control register to the new value
        asm volatile ("mov %0, %%cr4" :: "r" (bitmask));

        asm ("fninit");
}
