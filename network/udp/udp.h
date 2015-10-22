#ifndef _UDP_DEFS_H_
#define _UDP_DEFS_H_

#include "types.h"
#include "network/ip.h"

typedef struct
{
    uint16_t src_port;
    uint16_t dest_port;
    uint16_t len;
    uint16_t checksum;
} UDP_Header;

#endif