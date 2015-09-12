#ifndef _PRIV_EXT2_H_
#define _PRIV_EXT2_H_

#include "types.h"
#include "ext2_structs.h"
#include "ext2.h"

#define POOL_SIZE MB(2)

uint8_t*
_EXT2_ReadAddr(FileDescriptor *desc,
               uint64_t addr,
               uint32_t len);

EXT2_BlockGroupDescriptor*
_EXT2_GetBlockGroup(FileDescriptor *desc,
                    uint32_t block_index);

EXT2_Inode*
_EXT2_GetInode(FileDescriptor *desc,
               uint32_t inode_i);

uint32_t
_EXT2_ReadBlockData(FileDescriptor *desc,
                    uint32_t block_index,
                    uint32_t offset,
                    uint8_t *dest,
                    size_t size);

uint8_t*
_EXT2_GetBlockFromInode(FileDescriptor *desc,
                        EXT2_Inode *inode,
                        uint32_t index);

#endif