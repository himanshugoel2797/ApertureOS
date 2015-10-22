#ifndef _IPV4_H_
#define _IPV4_H_

#include "types.h"
#include "network/ip.h"

#define IPV4_VER 4

typedef struct
{
	uint32_t version : 4;
	uint32_t ihl : 4;
	uint32_t dscp : 6;
	uint32_t ecn : 2;
	uint16_t total_length;
	uint16_t ident;
	uint16_t flags : 3;
	uint16_t frag_off : 13;
	uint8_t ttl;
	uint8_t protocol;
	uint16_t hdr_checksum;
	uint8_t src_ip[4];
	uint8_t dest_ip[4];
}IPV4_Header;

#endif