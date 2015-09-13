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
_EXT2_CommitChanges(FileDescriptor *desc)
{
    EXT2_DriverData *data = (EXT2_DriverData*)desc->data;

    if(desc->write != NULL)
    {
        if(desc->write(data->last_read_addr/512, POOL_SIZE, data->memory_pool) == 0)
            return -1;
    }
    return 0;
}

uint32_t
_EXT2_WriteAddr(FileDescriptor *desc,
                uint64_t addr,
                uint32_t len,
                uint16_t *src)
{
    if(len > POOL_SIZE)return -1;

    EXT2_DriverData *data = (EXT2_DriverData*)desc->data;

    //Read from this part in memory
    desc->read(addr/512, POOL_SIZE, data->memory_pool);

    //Calculate the offset we should place the data at to write to the specified address
    uint32_t actualReadAddress = (addr/512) * 512;
    uint32_t offset = addr - actualReadAddress;

    //Setup the write
    memcpy(data->memory_pool + offset, src, len);

    //Push the data back to disk
    if(desc->write != NULL)
    {
        if(desc->write(addr/512, len, data->memory_pool) == 0)
        {
            //Cleanup the buffers if the write failed and force a read from disk for the next read
            data->last_read_addr = 0;
            memset(data->memory_pool, 0, len);
            return -1;
        }
        data->last_read_addr = actualReadAddress;
    }
    else
    {
        return -2;
    }

    return 0;
}

void
_EXT2_MarkBlockFree(FileDescriptor *desc,
                    uint32_t block)
{
    EXT2_DriverData *data = (EXT2_DriverData*)desc->data;
    uint32_t block_index = (block)/data->blocks_per_group;

    EXT2_BlockGroupDescriptor bgd;
    memcpy(&bgd, _EXT2_GetBlockGroup(desc, block_index), sizeof(EXT2_BlockGroupDescriptor));

    uint32_t block_index_base = block % data->blocks_per_group;

    //Read the inode bitmap block
    uint32_t *bitmap = _EXT2_ReadAddr(desc,
                                      bgd.block_usage_bitmap_addr * data->block_size,
                                      data->block_size);

    bitmap[block/32] &= ~(1 << (block % 32));

    _EXT2_CommitChanges(desc);
}

void
_EXT2_MarkInodeFree(FileDescriptor *desc,
                    uint32_t inode_i)
{
    EXT2_DriverData *data = (EXT2_DriverData*)desc->data;
    uint32_t block_index = (inode_i - 1)/data->inodes_per_group;

    EXT2_BlockGroupDescriptor bgd;
    memcpy(&bgd, _EXT2_GetBlockGroup(desc, block_index), sizeof(EXT2_BlockGroupDescriptor));

    uint32_t block_index_base = (inode_i - 1) % data->inodes_per_group;

    uint64_t address =
        (bgd.inode_table_start_addr * data->block_size) + (block_index_base * data->inode_size);

    //Read the inode bitmap block
    uint32_t *bitmap = _EXT2_ReadAddr(desc,
                                      bgd.inode_usage_bitmap_addr * data->block_size,
                                      data->block_size);

    bitmap[inode_i/32] &= ~(1 << (inode_i % 32));
    _EXT2_CommitChanges(desc);

    //Modify the entry in place and then commit the changes
    EXT2_Inode *inode = _EXT2_ReadAddr(desc, address, data->inode_size);
    inode->hard_link_count = 0;

    _EXT2_CommitChanges(desc);
}

void
_EXT2_ClearEntry(FileDescriptor *desc,
                 uint32_t dir_inode,
                 const char *file_name)
{
    EXT2_DriverData *data = (EXT2_DriverData*)desc->data;
    if(dir_inode != 0)
    {
        //Search the tree to find the entry for the file
        //Find the directory by traversing the tree
        EXT2_Inode inode2;
        memcpy(&inode2, _EXT2_GetInode(desc, dir_inode), sizeof(EXT2_Inode));

        uint32_t size = (((uint64_t)inode2.size_hi) << 32 | inode2.size_lo);

        if(inode2.type_perm >> 12 == EXT2_INODE_DIR && inode2.hard_link_count != 0)
        {

            for(int i = 0; i < size/data->block_size; i++)
            {

                EXT2_DirectoryEntry *dir = _EXT2_GetBlockFromInode(desc, &inode2, i, FALSE);
                if(dir == NULL)break;
                char entry_name[256];
                uint32_t s = 0;

                while(dir->name_len != 0)
                {
                    memset(entry_name, 0, 256);
                    memcpy(entry_name, dir->name, 256);
                    COM_WriteStr("%s\r\n", entry_name);
                    if(strncmp(entry_name, file_name, strlen(file_name)) == 0)
                    {
                        memset(dir, 0, dir->entry_size);
                        dir->entry_size = (size - s);
                        _EXT2_CommitChanges(desc);
                        i = size;
                        break;
                    }
                    s += dir->entry_size;
                    dir = (uint32_t)dir + dir->entry_size;
                }
            }
        }
    }
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
                        uint32_t index, bool del)
{

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

        if (i1_prev_index != inode->direct_block[12])
        {
            i1_prev_index = inode->direct_block[12];

            memcpy(i1_cache,
                   (uint32_t*)_EXT2_ReadAddr(desc,
                                             inode->direct_block[12] * data->block_size,
                                             data->block_size)
                   , data->block_size);
        }

        read_index = i1_cache[index - 12];

        //If deleting, mark the block table free when we're done freeing everything from it
        if(del && index == (data->f_indir_e_cnt + 12) - 1)
        {
            _EXT2_MarkBlockFree(desc, inode->direct_block[12]);
        }
    }
    else if (index < data->s_indir_e_cnt + data->f_indir_e_cnt + 12)
    {
        if(inode->direct_block[13] == 0)
            return NULL;

        uint32_t table_1_index = (index - block_index_limit_1)/data->f_indir_e_cnt;

        if(i2_1_prev_index != inode->direct_block[13])
        {
            i2_1_prev_index = inode->direct_block[13];

            memcpy(i2_1_cache,
                   (uint32_t*)_EXT2_ReadAddr(desc,
                                             inode->direct_block[13] * data->block_size,
                                             data->block_size)
                   , data->block_size);

        }
        uint32_t i = (index - block_index_limit_1) % data->f_indir_e_cnt;

        if(i2_2_prev_index != i2_1_cache[table_1_index])
        {
            i2_2_prev_index = i2_1_cache[table_1_index];
            memcpy(i2_2_cache,
                   (uint32_t*)_EXT2_ReadAddr(desc,
                                             i2_1_cache[table_1_index] * data->block_size,
                                             data->block_size)
                   , data->block_size);
        }

        read_index = i2_2_cache[i];

        if(del && (i == data->f_indir_e_cnt - 1))
        {
            _EXT2_MarkBlockFree(desc, i);
        }

        if(del && index == (data->s_indir_e_cnt + data->f_indir_e_cnt + 12) - 1)
        {
            _EXT2_MarkBlockFree(desc, inode->direct_block[13]);
        }
    }
    else if (index < data->t_indir_e_cnt + data->s_indir_e_cnt + data->f_indir_e_cnt + 12)
    {

    }

    if(read_index == 0)return NULL;

    if(!del)
    {
        return _EXT2_ReadAddr(desc,
                              read_index * data->block_size,
                              data->block_size);
    } else
    {
        _EXT2_MarkBlockFree(desc, read_index);
        return (uint8_t*)1;
    }
}

uint8_t
_EXT2_GetFileInfo(FileDescriptor *desc,
                  const char *filename,
                  bool *is_dir,
                  uint32_t *inode_num)
{
    EXT2_DriverData *data = (EXT2_DriverData*)desc->data;
    char *fname = filename + strlen(desc->path) - 1;
    char dir_name[256];

    uint32_t inode_i = ROOT_INODE_INDEX;
    uint64_t size = 0;
    uint32_t prev_inode = 0;

    //Maake sure the path is a directory
    while(fname != NULL)
    {
        memset(dir_name, 0, 256);
        uint32_t index = (uint32_t)strchr(fname + 1, '/') - (uint32_t)fname - 1;
        
        if(index == 0)
            break;

        memcpy(dir_name, fname + 1, (index > strlen(fname))?strlen(fname):index);

        //Find the directory by traversing the tree
        
        if(inode_i == prev_inode)
        {
            COM_WriteStr("PATH NOT FOUND\r\n");
            return -1;
        }

        EXT2_Inode inode;
        memcpy(&inode, _EXT2_GetInode(desc, inode_i), sizeof(EXT2_Inode));

        if(inode.hard_link_count == 0)return -1;

        size = (((uint64_t)inode.size_hi) << 32 | inode.size_lo);

        if(inode.type_perm >> 12 == EXT2_INODE_DIR)
        {

            for(int i = 0; i < size/data->block_size; i++)
            {

                EXT2_DirectoryEntry *dir = _EXT2_GetBlockFromInode(desc, &inode, i, FALSE);
                if(dir == NULL)
                    break;
                
                char entry_name[256];

                while(dir->name_len != 0)
                {
                    memset(entry_name, 0, 256);
                    memcpy(entry_name, dir->name, 256);
                    
                    prev_inode = inode_i;
                    if(strncmp(entry_name, dir_name, strlen(dir_name)) == 0)
                    {
                        inode_i = dir->inode_index;
                        if(strchr(fname + 1, '/') == NULL)
                        {
                            if(inode_num != NULL)*inode_num = inode_i;
                            if(is_dir != NULL)*is_dir = (dir->type == 2);
                            i = size;
                            fname = NULL;
                        }
                        break;
                    }
                    if(dir->entry_size == 0)break;
                    dir = (uint32_t)dir + dir->entry_size;
                }
            }
        }

        if(fname != NULL)
        {
            char* f_t = strchr(fname + 1, '/');
            if(f_t != NULL)fname = f_t;
        }
    }

    if(*inode_num == 0)
    {
        COM_WriteStr("FAILURE!!");
        return -1;
    }
    return 0;
}