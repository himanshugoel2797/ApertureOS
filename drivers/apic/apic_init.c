#include "apic.h"
#include "io_apic/io_apic.h"

#include "acpi_tables/acpi_tables.h"
#include "acpi_tables/madt.h"

uint8_t APIC_Initialize()
{
        //Ask ACPI for the MADT table
        MADT *madt = ACPITables_FindTable(MADT_SIG);

        uint32_t len = madt->h.Length - 8;
        for(uint32_t i = 0; i < len; i++)
        {
                switch(madt->entries[i])
                {
                case MADT_LAPIC_ENTRY_TYPE:
                {
                        MADT_EntryLAPIC *lapic = (MADT_EntryLAPIC*)&madt->entries[i];
                        i += sizeof(MADT_EntryLAPIC);
                        //We don't need to do anything with these yet
                }
                break;
                case MADT_IOAPIC_ENTRY_TYPE:
                {
                        MADT_EntryIOAPIC *ioapic = (MADT_EntryIOAPIC*)&madt->entries[i];
                        IOAPIC_Initialize(ioapic->io_apic_base_addr, ioapic->global_sys_int_base, ioapic->io_apic_id);
                        i+= sizeof(MADT_EntryIOAPIC);
                }
                break;
                case MADT_ISAOVER_ENTRY_TYPE:
                {
                        MADT_EntryISAOVR *isaovr = (MADT_EntryISAOVR*)&madt->entries[i];
                        IOAPIC_MapIRQ(isaovr->global_sys_int, isaovr->irq_src, APIC_GetID());
                        i+= sizeof(MADT_EntryISAOVR);
                }
                break;
                default:
                        break;
                }
        }

        return APIC_LocalInitialize() << 1 | 1;
}
