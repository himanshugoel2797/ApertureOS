#include "pci.h"
#include "pci_devices_names.h"

void 
pci_GetPCIClass3 (
    unsigned char baseid,
    unsigned char subid,
    unsigned char progid,
    char **   basedesc,
    char **   subdesc,
    char **   progdesc
)
{
    int i;

    *basedesc = *subdesc = *progdesc = "";

    for ( i=0; i < PCI_CLASSCODETABLE_LEN; ++i )
    {
        if ( PciClassCodeTable[i].BaseClass == baseid )
        {
            if ( !(**basedesc) )
                *basedesc = PciClassCodeTable[i].BaseDesc;
            if ( PciClassCodeTable[i].SubClass == subid )
            {
                if ( !(**subdesc) )
                    *subdesc = PciClassCodeTable[i].SubDesc;
                if ( PciClassCodeTable[i].ProgIf == progid )
                {
                    *progdesc = PciClassCodeTable[i].ProgDesc;
                    break;
                }
            }
        }
    }
}

void
pci_GetPCIClass (
    long classcode,
    char ** base,
    char ** sub,
    char ** prog
)
{
    unsigned char baseid, subid, progid;

    baseid  = ((classcode >> 24) & 0xFF);
    subid  =  ((classcode >> 16) & 0xFF);
    progid  = ((classcode >>  8) & 0xFF);

    pci_GetPCIClass3 (baseid,subid,progid,base,sub,prog);
}

void 
pci_GetPCIVendor(uint16_t venID,
                 char ** short_name,
                 char ** long_name)
{
    int i;
    *short_name = *long_name = "";

    for(i = 0; i < PCI_VENTABLE_LEN; ++i)
    {
        if(PciVenTable[i].VenId == venID)
        {
            if (!(**short_name))
                *short_name = PciVenTable[i].VenShort;

            if (!(**long_name))
                *long_name = PciVenTable[i].VenFull;
        }
    }
}

void
pci_GetPCIDevice(uint16_t venID,
                 uint16_t devID,
                 char **chip_name,
                 char **chip_desc)
{
    int i;
    *chip_name = *chip_desc = "";

    for(i = 0; i < PCI_DEVTABLE_LEN; ++i)
    {
        if(PciDevTable[i].VenId == venID && PciDevTable[i].DevId == devID)
        {
            if (!(**chip_name))
                *chip_name = PciDevTable[i].Chip;

            if (!(**chip_desc))
                *chip_desc = PciDevTable[i].ChipDesc;
        }
    }
}