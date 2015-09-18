#include "network.h"
#include "globals.h"
#include "drivers.h"
#include "utils/common.h"

NI_DriverInterface ni_drivers[] =
{
    {RTL8139_Detect, RTL8139_Initialize, FALSE, FALSE},
    {NULL, NULL, FALSE, FALSE}
};

uint8_t *ni_transmitBuffer, *ni_recieveBuffer;
uint32_t ni_transmitOff, ni_recieveOff, ni_presentDevices;

void
NI_Initialize(void)
{
    ni_presentDevices = 0;

    for(int i = 0; i < pci_deviceCount; i++)
        {
            int j = 0;
            while(ni_drivers[j].detect != NULL)
                {
                    //Only detect the first instance of each card
                    if(!ni_drivers[j].present && ni_drivers[j].detect)
                        {
                            ni_drivers[j].present = TRUE;
                            ni_presentDevices++;
                        }
                    j++;
                }
        }

}

void
NI_Start(void)
{
    //If there's only one active driver, activate it, otherwise we need to select
    if(ni_presentDevices == 0)
        {
            return;	//There's nothing to initialize
        }

    int j = 0;
    while(ni_drivers[j].detect != NULL)
        {
            //We will want to prefer wifi devices over ethernet devices, however we want to switch to ethernet if it is plugged in, one way would be to have each device detect if a connection is available
        }
}