#ifndef _IP_DEFS_H_
#define _IP_DEFS_H_

#include "types.h"

typedef struct{
 	uint8_t ip_addr_v4[4];
 	uint8_t ip_addr_v6[16];
 	uint8_t mac_addr[6];
}IP_t;

#endif