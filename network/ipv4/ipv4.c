#include "ipv4.h"
#include "utils/common.h"

static uint16_t ident = 0;

uint16_t
IPV4_CalculateChecksum(void *target,
                       int hdr_len)
{
    uint16_t *dat = (uint16_t*)target;

    uint32_t long_sum = 0;

    for(int i = 0; i < hdr_len/sizeof(uint16_t); i++)
        {
            long_sum += dat[i];
        }

    uint16_t checksum = (long_sum >> 16) + (uint16_t)(long_sum & 0xffff);
    return ~checksum;
}

void
IPV4_GeneratePacket(void *target,
                    IP_t *src,
                    IP_t *dst,
                    uint16_t length,
                    uint16_t fragment_count,
                    uint16_t curFragment_offset,
                    uint8_t protocol)
{
    IPV4_Header *hdr = (IPV4_Header*)target;
    hdr->version = IPV4_VER;
    hdr->ihl = sizeof(IPV4_Header)/sizeof(uint32_t);
    hdr->dscp = 0;
    hdr->ecn = 0;
    hdr->total_length = length;
    hdr->ident = ident++;
    hdr->flags = (fragment_count > 0);
    hdr->frag_off = curFragment_offset;
    hdr->ttl = 70;	//70 hops is a safe time to live
    hdr->protocol = protocol;
    hdr->hdr_checksum = 0;
    memcpy(hdr->src_ip, src->ip_addr_v4, 4);
    memcpy(hdr->dest_ip, dst->ip_addr_v4, 4);

    hdr->hdr_checksum = IPV4_CalculateChecksum(target, sizeof(IPV4_Header));
}