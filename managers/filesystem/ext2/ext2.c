#include "ext2.h"
#include "utils/common.h"
#include "kmalloc.h"

int index = 0;
int base_id = 0;

EXT2_FD *fd, *last_fd;

#define POOL_SIZE KB(128)

uint8_t* _EXT2_ReadAddr(FileDescriptor *desc, uint64_t addr, uint32_t len)
{
	EXT2_DriverData *data = (EXT2_DriverData*)desc->data;
	uint8_t *mem_pool = data->memory_pool;

	if(data->last_read_addr <= addr && data->last_read_addr + POOL_SIZE >= addr + len)
	{
		mem_pool = &mem_pool[(addr - data->last_read_addr)];
	}else{
		if(desc->read(addr/512, POOL_SIZE, (uint16_t*)mem_pool) < 0)return NULL;
		mem_pool += addr % 512;
		data->last_read_addr = (addr/512) * 512;
	}

	return mem_pool;
}

EXT2_BlockGroupDescriptor * _EXT2_GetBlockGroup(FileDescriptor *desc, uint32_t block_index)
{
	EXT2_DriverData *data = (EXT2_DriverData*)desc->data;

	uint32_t read_address = data->block_size;
	if(data->block_size == 1024)read_address += data->block_size;

	EXT2_BlockGroupDescriptor *bgdt = (EXT2_BlockGroupDescriptor*)_EXT2_ReadAddr(desc, read_address, KB(16));
	return &bgdt[block_index];
}

EXT2_Inode * _EXT2_GetInode(FileDescriptor *desc, uint32_t inode_i)
{
	EXT2_DriverData *data = (EXT2_DriverData*)desc->data;
	uint32_t block_index = (inode_i - 1)/data->inodes_per_group;

	EXT2_BlockGroupDescriptor bgd;
	memcpy(&bgd, _EXT2_GetBlockGroup(desc, block_index), sizeof(EXT2_BlockGroupDescriptor));

	uint32_t block_index_base = (inode_i - 1) % data->inodes_per_group;
	uint64_t address = (bgd.inode_table_start_addr * data->block_size) + (block_index_base * data->inode_size);

	return _EXT2_ReadAddr(desc, address, data->inode_size);
}

EXT2_FD * _EXT2_FindFDFromID(uint32_t id)
{
	EXT2_FD *cur_fd = fd;
	while(cur_fd->next != NULL)
	{
		if(cur_fd->id == id)break;
		cur_fd = cur_fd->next;
	}
	if(cur_fd->id == id)return cur_fd;
	return NULL;
}

uint32_t _EXT2_Initialize(FileDescriptor *desc)
{
	uint8_t *memory_pool = bootstrap_malloc(POOL_SIZE);
	if(memory_pool == NULL)return -1;

	if(desc->read(0, POOL_SIZE, (uint16_t*)memory_pool) < 0)return -2;

	EXT2_SuperBlock *s_blk = &memory_pool[1024];

	if(s_blk->ext2_sig != EXT2_SIG)return -1;

	EXT2_DriverData *data = kmalloc(sizeof(EXT2_DriverData));
	desc->data = data;
	data->memory_pool = memory_pool;
	data->base_block_num = s_blk->block_number;
	data->block_size = 1024 << s_blk->block_size_log2;
	data->major_version = s_blk->major_version;
	data->inode_size = (data->major_version)?s_blk->ext.sizeof_inode:128;
	data->blocks_per_group = s_blk->blocks_per_group;
	data->inodes_per_group = s_blk->inodes_per_group;
	data->inode_count = s_blk->inode_count;
	data->block_count = s_blk->block_count;
	data->group_count = s_blk->block_count/s_blk->blocks_per_group + 1;

	if(data->major_version)strcpy(data->vol_name, s_blk->ext.vol_name);
	else{
		sprintf(data->vol_name, "EXT2_%d", index++);
	}
	data->last_read_addr = 0;
	
	if(fd == NULL)fd = kmalloc(sizeof(EXT2_FD));
	last_fd = fd;

	fd->inode = 2;
	fd->is_directory = TRUE;
	fd->id = base_id++;

	return 0;
}


uint32_t _EXT2_Filesystem_OpenFile(FileDescriptor *desc, const char *filename, int flags, int perms)
{
	//Navigate the tree to find the inode for the file
	//Read the inode for the file to retrive its contents

	EXT2_DriverData *data = (EXT2_DriverData*)desc->data;
	char *fname = filename + strlen(desc->path) - 1;
	char dir_name[256];

	uint32_t inode_i = ROOT_INODE_INDEX;

	//Maake sure the path is a directory
	while(fname != NULL)
	{
		memset(dir_name, 0, 256);
		uint32_t index = (uint32_t)strchr(fname + 1, '/') - (uint32_t)fname - 1;
		if(index == 0)break;
		if(index > strlen(fname))
		{
			memcpy(dir_name, fname + 1, strlen(fname) - 1);
		}
		else
		{
			memcpy(dir_name, fname + 1, index);
		}

		//Find the directory by traversing the tree
		EXT2_Inode inode;
		memcpy(&inode, _EXT2_GetInode(desc, inode_i), sizeof(EXT2_Inode));

		for(int i = 0; i < 12; i++)
		{
			if(inode.direct_block[i] == 0)break;

			//Check the block entries
			uint64_t address = inode.direct_block[i] * data->block_size;

			if(inode.type_perm >> 12 == EXT2_INODE_DIR)
			{
				EXT2_DirectoryEntry *dir = 	_EXT2_ReadAddr(desc, address, 512);
				char entry_name[256];

				while(dir->name_len != 0)
				{
					memset(entry_name, 0, 256);
					memcpy(entry_name, dir->name, 256);
					if(strncmp(entry_name, dir_name, strlen(dir_name)) == 0){
						inode_i = dir->inode_index;
						i = 13;
						fname = NULL;
						break;
					}
					dir = (uint32_t)dir + dir->entry_size;
				}
			}	
		}

		if(fname != NULL)fname = strchr(fname + 1, '/');
	}

	EXT2_FD* fd_n = kmalloc(sizeof(EXT2_FD));
	fd_n->id = base_id++;
	fd_n->is_directory = FALSE;
	fd_n->inode = inode_i;
	fd_n->next = NULL;
	fd_n->extra_info = 0;

	COM_WriteStr("inode: %d\r\n", inode_i);

	last_fd->next = fd_n;
	last_fd = last_fd->next;

	return fd_n->id;

}

uint8_t _EXT2_Filesystem_SeekFile(FileDescriptor *desc, uint32_t fd, uint32_t offset, int whence)
{
	//Seek for a part of the file
}

uint8_t _EXT2_Filesystem_CloseFile(FileDescriptor *desc, uint32_t fd)
{
	//Remove the entry for the file
}

uint8_t _EXT2_Filesystem_DeleteFile(FileDescriptor *desc, const char *file)
{

}

uint8_t _EXT2_Filesystem_RenameFile(FileDescriptor *desc, const char *orig_name, const char *new_name)
{

}

uint32_t _EXT2_Filesystem_OpenDir(FileDescriptor *desc, const char *filename)
{
	EXT2_DriverData *data = (EXT2_DriverData*)desc->data;
	char *fname = filename + strlen(desc->path) - 1;
	char dir_name[256];

	uint32_t inode_i = ROOT_INODE_INDEX;
	
	if(filename[strlen(filename) - 1] != '/')return -1;	//Make sure this is  directory path

	//Maake sure the path is a directory
	while(fname != NULL)
	{
		memset(dir_name, 0, 256);
		uint32_t index = (uint32_t)strchr(fname + 1, '/') - (uint32_t)fname - 1;
		if(index == 0)break;
		if(index > strlen(fname))break;
		memcpy(dir_name, fname + 1, index);

		//Find the directory by traversing the tree
		EXT2_Inode inode;
		memcpy(&inode, _EXT2_GetInode(desc, inode_i), sizeof(EXT2_Inode));

		for(int i = 0; i < 12; i++)
		{
			if(inode.direct_block[i] == 0)break;

			//Check the block entries
			uint64_t address = inode.direct_block[i] * data->block_size;

			if(inode.type_perm >> 12 == EXT2_INODE_DIR)
			{
				EXT2_DirectoryEntry *dir = 	_EXT2_ReadAddr(desc, address, 512);
				char entry_name[256];

				while(dir->name_len != 0)
				{
					memset(entry_name, 0, 256);
					memcpy(entry_name, dir->name, 256);
					if(dir->type == EXT2_DIRT_DIR && strncmp(entry_name, dir_name, strlen(dir_name)) == 0){
						inode_i = dir->inode_index;
						i = 13;
						fname = NULL;
						break;
					}
					dir = (uint32_t)dir + dir->entry_size;
				}
			}	
		}

		if(fname != NULL)fname = strchr(fname + 1, '/');
	}

	EXT2_FD* fd_n = kmalloc(sizeof(EXT2_FD));
	fd_n->id = base_id++;
	fd_n->is_directory = TRUE;
	fd_n->inode = inode_i;
	fd_n->next = NULL;
	fd_n->extra_info = 0;

	last_fd->next = fd_n;
	last_fd = last_fd->next;

	return fd_n->id;
}

uint8_t _EXT2_Filesystem_ReadDir(FileDescriptor *desc, uint32_t dd, Filesystem_DirEntry *dirent)
{
	//provide the next entry in the directory
	EXT2_DriverData *data = (EXT2_DriverData*)desc->data;
	EXT2_FD *cur_fd = _EXT2_FindFDFromID(dd);

	//Determine the address of the inode
	EXT2_Inode inode;
	memcpy(&inode, _EXT2_GetInode(desc, cur_fd->inode), sizeof(EXT2_Inode));
	uint32_t cur_index = 0;
	
	for(int i = 0; i < 12; i++)
	{
		//COM_WriteStr("%d\r\n", i);
		if(inode.direct_block[i] == 0)break;

			//Check the block entries
		uint64_t address = inode.direct_block[i] * data->block_size;

		EXT2_DirectoryEntry *dir = 	_EXT2_ReadAddr(desc, address, 512);

		uint32_t traversed_size = 0;

		while(traversed_size < data->block_size)
		{
			if(cur_index == cur_fd->extra_info){
			
				memset(dirent->dir_name, 0, 256);
				memcpy(dirent->dir_name, dir->name, dir->name_len);
				dirent->name_len = strlen(dirent->dir_name);
				dirent->type = dir->type;
				i = 13;

				cur_fd->extra_info++;
				return 0;
			}
			cur_index++;

			traversed_size += dir->entry_size;
			dir = (uint32_t)dir + dir->entry_size;
		}
	}	
	return -1;
}

uint8_t _EXT2_Filesystem_CloseDir(FileDescriptor *desc, uint32_t fd)
{
	//Find and remove the associated entry

}

uint8_t _EXT2_Filesystem_MakeDir(FileDescriptor *desc, const char *path)
{
	
}

uint8_t _EXT2_Filesystem_DeleteDir(FileDescriptor *desc, const char *path)
{

}