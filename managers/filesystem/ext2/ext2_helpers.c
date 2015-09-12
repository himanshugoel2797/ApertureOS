#include "priv_ext2.h"

uint8_t*
_EXT2_ReadAddr(FileDescriptor *desc,
               uint64_t addr,
               uint32_t len)
{
    EXT2_DriverData *data = (EXT2_DriverData*)desc->data;
    uint8_t *mem_pool = data->memory_pool;

    if(data->last_read_addr <= addr &&
            (data->last_read_addr + POOL_SIZE) >= (addr + len))
        {
            mem_pool = &mem_pool[(addr - data->last_read_addr)];
        }
    else
        {
            if(desc->read(addr/512, POOL_SIZE, (uint16_t*)mem_pool) == 0)return NULL;
            mem_pool += addr % 512;
            data->last_read_addr = (addr/512) * 512;
        }

    return mem_pool;
}

uint32_t
_EXT2_WriteAddr(FileDescriptor *desc,
                uint64_t addr,
                uint32_t len,
                uint16_t *src)
{
	if(len > POOL_SIZE)return -1;

	EXT2_DriverData *data = (EXT2_DriverData*)desc->data;
	memcpy(data->memory_pool, src, len);

	if(desc->write != NULL)
	{
		if(desc->write(addr/512, len, data->memory_pool) == 0)
		{
			//Cleanup the buffers if the write failed and force a read from disk for the next read
			data->last_read_addr = 0;
			memset(data->memory_pool, 0, len);
			return -1;
		}
		data->last_read_addr = (addr/512) * 512;
	}
	else
	{
		return -2;
	}

	return 0;
}



EXT2_BlockGroupDescriptor*
_EXT2_GetBlockGroup(FileDescriptor *desc,
                    uint32_t block_index)
{
    EXT2_DriverData *data = (EXT2_DriverData*)desc->data;

    uint32_t read_address = data->block_size;
    if(data->block_size == 1024)read_address += data->block_size;

    EXT2_BlockGroupDescriptor *bgdt = (EXT2_BlockGroupDescriptor*)_EXT2_ReadAddr(desc, 
                                                                                 read_address, 
                                                                                 KB(16));
    return &bgdt[block_index];
}

EXT2_Inode*
_EXT2_GetInode(FileDescriptor *desc,
               uint32_t inode_i)
{
    EXT2_DriverData *data = (EXT2_DriverData*)desc->data;
    uint32_t block_index = (inode_i - 1)/data->inodes_per_group;

    EXT2_BlockGroupDescriptor bgd;
    memcpy(&bgd, _EXT2_GetBlockGroup(desc, block_index), sizeof(EXT2_BlockGroupDescriptor));

    uint32_t block_index_base = (inode_i - 1) % data->inodes_per_group;
    
    uint64_t address = 
    (bgd.inode_table_start_addr * data->block_size) + (block_index_base * data->inode_size);
    
    return _EXT2_ReadAddr(desc, address, data->inode_size);
}

uint32_t
_EXT2_ReadBlockData(FileDescriptor *desc,
                    uint32_t block_index,
                    uint32_t offset,
                    uint8_t *dest,
                    size_t size)
{
    EXT2_DriverData *data = (EXT2_DriverData*)desc->data;
    uint32_t block_address = block_index * data->block_size;
    memcpy(dest, _EXT2_ReadAddr(desc, block_address, size + offset) + offset, size);
    return size;
}

uint8_t*
_EXT2_GetBlockFromInode(FileDescriptor *desc,
                        EXT2_Inode *inode,
                        uint32_t index)
{

	//TODO optimize this by using a cache for the indirection tables
	
	//Determine what the block index for the requested index is
	EXT2_DriverData *data = (EXT2_DriverData*)desc->data;

	uint32_t read_index = 0;
    uint32_t block_index_limit_1 = data->f_indir_e_cnt + 12;
    uint32_t block_index_limit_2 = block_index_limit_1 + data->s_indir_e_cnt;

	if (index < 12)
	{
		//Stop, this block is empty
		if (inode->direct_block[index] == 0)
			return NULL;

		read_index = inode->direct_block[index];
	}
	else if (index < data->f_indir_e_cnt + 12)
	{
		if (inode->direct_block[12] == 0)
			return NULL;

		uint32_t *ind_t_1 = (uint32_t*)_EXT2_ReadAddr(desc,
		                                              inode->direct_block[12] * data->block_size,
		                                              data->block_size);

		read_index = ind_t_1[index - 12];
	}
	else if (index < data->s_indir_e_cnt + data->f_indir_e_cnt + 12)
	{
		if(inode->direct_block[13] == 0)
			return NULL;

		uint32_t table_1_index = (index - block_index_limit_1)/data->f_indir_e_cnt;

		uint32_t *ind_t_1 = (uint32_t*)_EXT2_ReadAddr(desc,
		                                              inode->direct_block[13] * data->block_size,
		                                              data->block_size);

		uint32_t i = (index - block_index_limit_1) % data->f_indir_e_cnt;

		ind_t_1 = (uint32_t*)_EXT2_ReadAddr(desc,
		                                   	ind_t_1[table_1_index] * data->block_size,
		                                    data->block_size);

		read_index = ind_t_1[i];
	}
	else if (index < data->t_indir_e_cnt + data->s_indir_e_cnt + data->f_indir_e_cnt + 12)
	{

	}

	if(read_index == 0)return NULL;

	return _EXT2_ReadAddr(desc,
	               read_index * data->block_size,
	               data->block_size);
}