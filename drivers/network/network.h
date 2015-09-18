#ifndef _NETWORK_INTERFACE_DRIV_H_
#define _NETWORK_INTERFACE_DRIV_H_

#include "types.h"
#include "rtl8139/rtl8139.h"

//* Provides a means for a network device to be interfaced without regard for which device it is
typedef struct
{
    bool (*detect)(uint32_t);				//*< Check if this network device is present
    uint32_t (*init)(uint32_t);				//*< Initialize the network device
    uint32_t (*enable)(uint32_t);			//*< Enable the network device
    uint32_t (*disable)(uint32_t);			//*< Disable the network device
    uint32_t (*set_power_state)(uint32_t, 	//*< Set the power state for the network device
                                uint32_t);

    bool active;
    bool present;
} NI_DriverInterface;

//* Request a pointer to the packet transmission ring buffer

//* \return pointer to the packet transmission ring buffer
//* \sa NI_RequestRecievePointer()
uint8_t*
NI_RequestTransmitPointer(void);

//* Request a pointer to the packet recieve ring buffer

//* \return pointer to the packet recieve ring buffer
//* \sa NI_RequestTransmitPointer()
uint8_t*
NI_RequestRecievePointer(void);

//* Initialize the network interface

//* \sa NI_RequestTransmitPointer(), NI_RequestRecievePointer()
void
NI_Initialize(void);

//* Start the network interface

//* \sa NI_Initialize()
void
NI_Start(void);

#endif