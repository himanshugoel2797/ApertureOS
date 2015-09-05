#include "cpuid.h"

uint32_t ecx = 0, edx = 0;

uint8_t CPUID_FeatureIsAvailable(uint8_t cpuid_register, CPUID_FEAT feature)
{
    if(ecx == 0 | edx == 0) asm volatile ("cpuid\n\t" : "=c" (ecx), "=d" (edx) : "a" (1) : "ebx" );

    if(cpuid_register == CPUID_ECX)
    {
        return (ecx & (uint32_t)feature) == (uint32_t)feature;
    }
    else if(cpuid_register == CPUID_EDX)
    {
        return (edx & (uint32_t)feature) == (uint32_t)feature;
    }
    return -1;
}
