#include "apic.h"
#include "io_apic/io_apic.h"

#include "acpi_tables/acpi_tables.h"
#include "acpi_tables/madt.h"

#include "drivers.h"
#include "cpuid.h"

uint32_t APIC_Initialize()
{
        //Initialize the local APIC
        uint8_t apic_available = CPUID_FeatureIsAvailable(CPUID_EDX, CPUID_FEAT_EDX_APIC);
        if(!apic_available) return -1;

        //Initialize the local APIC
        APIC_LocalInitialize();

        //Ask ACPI for the MADT table
        MADT *madt = ACPITables_FindTable(MADT_SIG);

        if(madt != NULL) {
                uint32_t len = madt->h.Length - 8;
                for(uint32_t i = 0; i < len; i++)
                {
                        switch(madt->entries[i])
                        {
                        case MADT_LAPIC_ENTRY_TYPE:
                        {
                                MADT_EntryLAPIC *lapic = (MADT_EntryLAPIC*)&madt->entries[i];
                                i += lapic->h.entry_size - 1;
                                //We don't need to do anything with these yet
                                COM_WriteStr("\r\nLAPIC\r\n");
                                COM_WriteStr("\tLen: %d\r\n", lapic->h.entry_size);
                                COM_WriteStr("\tID: %u\r\n", lapic->apic_id);
                                COM_WriteStr("\tProcessor ID: %x", lapic->processor_id);
                        }
                        break;
                        case MADT_IOAPIC_ENTRY_TYPE:
                        {
                                MADT_EntryIOAPIC *ioapic = (MADT_EntryIOAPIC*)&madt->entries[i];
                                i+= ioapic->h.entry_size  - 1;

                                COM_WriteStr("\r\nIOAPIC\r\n");
                                COM_WriteStr("\tLen: %d\r\n", ioapic->h.entry_size);
                                COM_WriteStr("\tID: %x\r\n", ioapic->io_apic_id);
                                COM_WriteStr("\tBase Address: %x\r\n", ioapic->io_apic_base_addr);
                                COM_WriteStr("\tGlobal Interrupt Base: %x\r\n", ioapic->global_sys_int_base);

                                IOAPIC_Initialize(ioapic->io_apic_base_addr, ioapic->global_sys_int_base, ioapic->io_apic_id);

                                //Map IRQ 0-15 to 32-47
                                for(int j = 0; j < 16; j++) {
                                        IOAPIC_MapIRQ(j, 32 + j, APIC_GetID(), 0, 0);
                                }
                        }
                        break;
                        case MADT_ISAOVER_ENTRY_TYPE:
                        {
                                MADT_EntryISAOVR *isaovr = (MADT_EntryISAOVR*)&madt->entries[i];
                                i+= isaovr->h.entry_size - 1;

                                int polarity = isaovr->flags & 3;
                                int triggerMode = (isaovr->flags >> 2) & 3;

                                IOAPIC_MapIRQ(isaovr->global_sys_int, 32 + isaovr->irq_src, APIC_GetID(), triggerMode >> 1,polarity >> 1);

                                COM_WriteStr("\r\nISAOVR\r\n");
                                COM_WriteStr("\tLen: %d\r\n", isaovr->h.entry_size);
                                COM_WriteStr("\tIRQ: %d\r\n", isaovr->irq_src);
                                COM_WriteStr("\tBus: %d\r\n", isaovr->bus_src);
                                COM_WriteStr("\tGlobal Interrupt: %d\r\n", isaovr->global_sys_int);
                        }
                        break;
                        default:
                                COM_WriteStr("Unknown Type %d\r\n", madt->entries[i]);
                                break;
                        }
                        if( ((MADT_EntryHeader*)&madt->entries[i])->entry_size == 0) break;
                }
        }

        return 0;
}
