#ifndef _PRIV_GPT_STRUCTS_H_
#define _PRIV_GPT_STRUCTS_H_

#include "types.h"

#define GPT_MAGIC_VAL 0x5452415020494645

typedef struct{
 	uint64_t gpt_magic;
 	uint32_t revision;
 	uint32_t header_size;
 	uint32_t header_crc;
 	uint32_t rsv0;
 	uint64_t cur_lba;
 	uint64_t backup_lba;
 	uint64_t first_usable_lba;
 	uint64_t last_usable_lba;
 	uint8_t disk_guid[16];
 	uint64_t lba_partition_entries;
 	uint32_t partition_entry_count;
 	uint32_t partition_entry_size;
 	uint32_t partition_entries_crc;
}GPT_Header;

typedef struct{
	uint8_t partition_type_guid[16];
	uint8_t uniq_part_guid[16];
	uint64_t start_lba;
	uint64_t last_lba;
	uint64_t flags;
	uint16_t name[36];
}GPT_Entry;


#endif