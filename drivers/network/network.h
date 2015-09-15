#ifndef _NETWORK_INTERFACE_DRIV_H_
#define _NETWORK_INTERFACE_DRIV_H_

#include "types.h"

#include "rtl8139/rtl8139.h"

//* Provides a means for a network device to be interfaced without regard for which device it is
typedef struct{
	bool (*detect)(void);	//*< Check if this network device is present
}NI_DriverInterface;

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

#endif