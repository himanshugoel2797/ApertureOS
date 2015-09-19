#ifndef PRIV_MBR_STRUCTS
#define PRIV_MBR_STRUCTS

#include "types.h"

#define MBR_SIG_1 0xAA
#define MBR_SIG_0 0x55

typedef struct{
	uint8_t status;
	uint8_t chs_s[3];
	uint8_t partition_type;
	uint8_t chs_e[3];
	uint32_t lba_start;
	uint32_t sector_count;
}MBR_PARTITION_ENTRY;

typedef struct{
	uint8_t bootstrap[446];
	MBR_PARTITION_ENTRY partitions[4];
	uint8_t boot_sig[2];
}__attribute__((packed)) MBR_HEADER;

#endif