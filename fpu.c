#include "fpu.h"
#include "cpuid.h"
#include "interruptmanager.h"

void FPU_StateHandler(Registers *regs);

void FPU_Initialize()
{
        if(!CPUID_FeatureIsAvailable(CPUID_EDX, CPUID_FEAT_EDX_FPU)) {
                //OS can't load
                return;
        }
        if(!CPUID_FeatureIsAvailable(CPUID_EDX, CPUID_FEAT_EDX_SSE)) {
                //OS can't load
                return;
        }
        if(!CPUID_FeatureIsAvailable(CPUID_ECX, CPUID_FEAT_ECX_XSAVE)) {
                //OS can't load
                return;
        }

        uint32_t bitmask = 0;
        //Get the current control register 0 value
        asm volatile ("mov %%cr0, %0" : "=r" (bitmask));

        bitmask &= ~(1 << 2); //Enable the FPU
        bitmask |= (1 << 5); //Enable FPU exceptions
        bitmask |= (1 << 1); //FWAIT should cause a state change

        //Set the control register to the new value
        asm volatile ("mov %0, %%cr0" :: "r" (bitmask));

        //Get the current control register 0 value
        asm volatile ("mov %%cr4, %0" : "=r" (bitmask));

        bitmask |= (1 << 9); //Enable FPU exceptions
        bitmask |= (1 << 10); //Allow CPU to backup FPU state
        bitmask |= (1 << 18); //FWAIT should cause a state change

        //Set the control register to the new value
        asm volatile ("mov %0, %%cr4" :: "r" (bitmask));

        InterruptManager_RegisterHandler(7, 0, FPU_StateHandler);

        asm ("fninit");
}

void FPU_EnableInterrupts()
{
        uint32_t bitmask = 0;
        //Get the current control register 0 value
        asm volatile ("mov %%cr0, %0" : "=r" (bitmask));
        bitmask |= (1 << 3); //Allow CPU to backup FPU state
        //Set the control register to the new value
        asm volatile ("mov %0, %%cr0" :: "r" (bitmask));
}

void FPU_StateHandler(Registers *regs)
{
        //TODO FXSSAVE
        asm volatile ("clts");
}
