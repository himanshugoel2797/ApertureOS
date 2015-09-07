#include "ext2.h"
#include "kmalloc.h"

uint8_t *memory_pool = NULL;
int index = 0;

uint8_t* _EXT2_ReadAddr(FileDescriptor *desc, uint64_t addr, uint32_t len)
{
	uint8_t mem_pool = memory_pool;
	EXT2_DriverData *data = (EXT2_DriverData*)desc->data;

	if(data->last_read_addr <= addr && data->last_read_addr + KB(32) >= addr + len)
	{
		mem_pool = mem_pool + (addr - data->last_read_addr);
	}else{
		if(desc->read(addr/512, KB(32), (uint16_t*)mem_pool) < 0)return NULL;
		mem_pool += addr % 512;
	}

	return mem_pool;
}


uint32_t _EXT2_Initialize(FileDescriptor *desc)
{
	if(memory_pool == NULL)memory_pool = bootstrap_malloc(KB(32));
	if(memory_pool == NULL)return -1;

	if(desc->read(0, KB(32), (uint16_t*)memory_pool) < 0)return -2;

	EXT2_SuperBlock *s_blk = &memory_pool[1024];

	if(s_blk->ext2_sig != EXT2_SIG)return -1;

	EXT2_DriverData *data = kmalloc(sizeof(EXT2_DriverData));
	desc->data = data;
	
	data->base_block_num = s_blk->block_number;
	data->block_size = 1024 << s_blk->block_size_log2;
	data->major_version = s_blk->major_version;
	data->inode_size = (data->major_version)?s_blk->ext.sizeof_inode:128;
	data->blocks_per_group = s_blk->blocks_per_group;
	data->inodes_per_group = s_blk->inodes_per_group;
	data->inode_count = s_blk->inode_count;
	data->block_count = s_blk->block_count;
	data->group_count = s_blk->block_count/s_blk->blocks_per_group + 1;

	memcpy(data->vol_name, data->major_version?s_blk->ext.vol_name:sprintf("EXT2_%d", index++), 16);
	data->last_read_addr = 0;
}


uint32_t _EXT2_Filesystem_OpenFile(FileDescriptor *desc, const char *filename, int flags, int perms)
{

}

uint8_t _EXT2_Filesystem_SeekFile(FileDescriptor *desc, uint32_t fd, uint32_t offset, int whence)
{

}

uint8_t _EXT2_Filesystem_CloseFile(FileDescriptor *desc, uint32_t fd)
{

}

uint8_t _EXT2_Filesystem_DeleteFile(FileDescriptor *desc, const char *file)
{

}

uint8_t _EXT2_Filesystem_RenameFile(FileDescriptor *desc, const char *orig_name, const char *new_name)
{

}

uint32_t _EXT2_Filesystem_OpenDir(FileDescriptor *desc, const char *filename)
{

}

uint8_t _EXT2_Filesystem_ReadDir(FileDescriptor *desc, uint32_t dd, Filesystem_DirEntry *dir)
{

}

uint8_t _EXT2_Filesystem_CloseDir(FileDescriptor *desc, uint32_t fd)
{

}

uint8_t _EXT2_Filesystem_MakeDir(FileDescriptor *desc, const char *path)
{
	
}

uint8_t _EXT2_Filesystem_DeleteDir(FileDescriptor *desc, const char *path)
{

}