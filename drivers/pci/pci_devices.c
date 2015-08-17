#include "pci.h"
#include "pci_devices_names.h"

void pci_GetPCIClass3 (
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
