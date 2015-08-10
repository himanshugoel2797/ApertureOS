#include "apic.h"
#include "priv_apic.h"
#include "pic/pic.h"

#include "cpuid.h"
#include "serial/COM.h"

#include "utils/common.h"
#include "utils/native.h"

uint8_t APIC_Initialize()
{
        PIC_Initialize(); //Initialize the PIC
        PIC_MaskAll();    //Disable all interrupts from it

        //Initialize the local APIC
        uint8_t apic_available = CPUID_FeatureIsAvailable(CPUID_EDX, CPUID_FEAT_EDX_APIC);
        uint8_t x2apic_available = CPUID_FeatureIsAvailable(CPUID_ECX, CPUID_FEAT_ECX_x2APIC);

        if(!apic_available) return -1;
        if(!x2apic_available) return -2;


        uint64_t apic_base_msr = rdmsr(IA32_APIC_BASE);
        apic_base_msr |= (3 << 10); //Enable both apic and x2apic mode
        wrmsr(IA32_APIC_BASE, apic_base_msr);

        //Set the spurious vector
        uint64_t svr = rdmsr(APIC_SVR);
        svr |= 0xFF;
        wrmsr(APIC_SVR, svr);

        //Software enable the APIC by setting the msr
        APIC_SetEnableMode(1);

        return 0;
}

void APIC_SetEnableMode(uint8_t enabled)
{
        uint64_t svr = rdmsr(APIC_SVR);
        svr = SET_VAL_BIT(svr, 8, enabled & 1);
        wrmsr(APIC_SVR, svr);
}
