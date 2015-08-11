#include "io_apic.h"
#include "priv_io_apic.h"

IOAPIC_Desc ioapics[MAX_IOAPIC_COUNT];
uint32_t curIOAPIC_index = 0;

uint8_t IOAPIC_Initialize(uint32_t baseAddr, uint32_t global_int_base, uint32_t id)
{
        if(curIOAPIC_index >= MAX_IOAPIC_COUNT) return -1;
        ioapics[curIOAPIC_index].baseAddr = baseAddr;
        ioapics[curIOAPIC_index].global_int_base = global_int_base;
        ioapics[curIOAPIC_index].ID = id;
        ioapics[curIOAPIC_index].entry_count = (IOAPIC_Read(baseAddr, 0x01) >> 16) & 0xFF;
        curIOAPIC_index++;
        return 0;
}

uint32_t IOAPIC_Read(uint32_t* io_apic_baseAddr, uint32_t index)
{
        io_apic_baseAddr[0] = index;
        return io_apic_baseAddr[4];
}

void IOAPIC_Write(uint32_t* io_apic_baseAddr, uint32_t index, uint32_t val)
{
        io_apic_baseAddr[0] = index;
        io_apic_baseAddr[4] = val;
}

void IOAPIC_MapIRQ(uint8_t global_irq, uint8_t apic_vector, uint64_t apic_id, uint8_t trigger_mode, uint8_t polarity)
{
        uint32_t irq_pin = 0;
        uint32_t *baseAddr = NULL;
        //Determine which APIC this should map to
        for(uint32_t i = 0; i < curIOAPIC_index; i++)
        {
                if(global_irq > ioapics[i].global_int_base && global_irq < (ioapics[i].global_int_base + ioapics[i].entry_count)) {
                        //Found the IO APIC to map to
                        irq_pin = global_irq - ioapics[i].global_int_base;
                        baseAddr = (uint32_t*)ioapics[i].baseAddr;
                }
        }
        if(baseAddr == NULL) return; //No match found!

        const uint32_t low_index = 0x10 + irq_pin*2;
        const uint32_t high_index = 0x10 + irq_pin*2 + 1;

        uint32_t high = IOAPIC_Read(baseAddr, high_index);
        // set APIC ID
        high &= ~0xff000000;
        high |= apic_id << 24;
        IOAPIC_Write(baseAddr, high_index, high);

        uint32_t low = IOAPIC_Read(baseAddr, low_index);

        // unmask the IRQ
        low &= ~(1<<16);
        low &= ~(1<<13);
        low |= ((polarity & 1) << 13);

        low &= ~(1<<15);
        low |= ((polarity & 1) << 15);

        // set to physical delivery mode
        low &= ~(1<<11);

        // set to fixed delivery mode
        low &= ~0x700;

        // set delivery vector
        low &= ~0xff;
        low |= apic_vector;

        IOAPIC_Write(baseAddr, low_index, low);
}
