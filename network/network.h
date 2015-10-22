#ifndef _NETWORK_STACK_H_
#define _NETWORK_STACK_H_

#include "types.h"
#include "drivers.h"
#include "ipv4/ipv4.h"
#include "udp/udp.h"

//* Initialize the network stack

void
NetworkStack_Init(void);

#endif