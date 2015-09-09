#ifndef _EXT2_FS_STRUCTS_H_
#define _EXT2_FS_STRUCTS_H_

#include "types.h"

#define EXT2_SIG 0xEF53
#define ROOT_INODE_INDEX 0x2

#define GET_BLOCK_INDEX_FROM_INODE(inode_index, inodes_per_group) ( (inode_index - 1)/inodes_per_group )
#define GET_BLOCK_INODE_INDEX_FROM_INODE(inode_index, inodes_per_group) ( (inode_index - 1)%inodes_per_group )


typedef enum{
	EXT2_INODE_FILE = 0x8,
	EXT2_INODE_DIR = 0x4
}EXT2_INODE_EntryTypes;

typedef enum{
	EXT2_DIRT_FILE = 1,
	EXT2_DIRT_DIR = 2
}EXT2_DIR_TABLE_EntryTypes;

typedef struct{
	uint32_t first_non_reserved_inode;
	uint16_t sizeof_inode;
	uint16_t block_guid_superblock;
	uint32_t optional_features;
	uint32_t required_features;
	uint32_t read_only_features;
	uint8_t fs_id[16];
	uint8_t vol_name[16];
	uint8_t last_mount_path[64];
	uint32_t compression;
	uint8_t blocks_prealloc_files;
	uint8_t blocks_prealloc_dirs;
	uint16_t unused;
	uint8_t journal_id[16];
	uint32_t journal_inode;
	uint32_t journal_device;
	uint32_t head_orphan_inode;
}EXT2_SuperBlock_Ext;

typedef struct
{
	uint32_t inode_count;
	uint32_t block_count;
	uint32_t superuser_blocks;
	uint32_t unallocated_block_count;
	uint32_t unallocated_inode_count;
	uint32_t block_number;
	uint32_t block_size_log2;	//Shift 1024 to the left by this to obtain the block size
	uint32_t fragment_size_log2;//Shift 1024 to the left by this to obtain the fragment size
	uint32_t blocks_per_group;
	uint32_t fragments_per_group;
	uint32_t inodes_per_group;
	uint32_t last_mount_time;
	uint32_t last_write_time;
	uint16_t num_mounted_since_check;
	uint16_t num_mounts_max;
	uint16_t ext2_sig;
	uint16_t fs_state;
	uint16_t error_handling;
	uint16_t minor_version;
	uint32_t last_check_time;
	uint32_t interval_forced_checks;
	uint32_t os_id;
	uint32_t major_version;
	uint16_t uid_reserved_blocks;
	uint16_t guid_reserved_blocks;
	EXT2_SuperBlock_Ext ext;
}EXT2_SuperBlock;

typedef struct{
	uint32_t block_usage_bitmap_addr;
	uint32_t inode_usage_bitmap_addr;
	uint32_t inode_table_start_addr;
	uint16_t num_unallocated_blocks;
	uint16_t num_unallocated_inodes;
	uint16_t num_dirs;
	uint8_t unused[32 - 18];
}EXT2_BlockGroupDescriptor;

typedef struct{
	uint16_t type_perm;
	uint16_t uid;
	uint32_t size_lo;
	uint32_t last_access_time;
	uint32_t creation_time;
	uint32_t last_mod_time;
	uint32_t deletion_time;
	uint16_t guid;
	uint16_t hard_link_count;
	uint32_t disk_sector_count;
	uint32_t flags;
	uint32_t os_specific;
	uint32_t direct_block[15];
	uint32_t generation_num;
	uint32_t extended_attr_block;
	uint32_t size_hi;
	uint32_t fragment_block_addr;
	uint8_t os_specific_2[12];
}EXT2_Inode;

typedef struct EXT2_DriverData_T{
	uint32_t base_block_num;
	uint32_t block_size;
	uint32_t inode_size;
	uint32_t blocks_per_group;
	uint32_t inodes_per_group;
	uint32_t inode_count;
	uint32_t block_count;
	uint32_t group_count;
	uint32_t major_version;
	uint8_t vol_name[16];
	uint64_t last_read_addr;
	uint8_t *memory_pool;
}EXT2_DriverData;

typedef struct
{
	uint32_t inode_index;
	uint16_t entry_size;
	uint8_t name_len;
	uint8_t type;
	uint8_t name[1];
}EXT2_DirectoryEntry;

#endif