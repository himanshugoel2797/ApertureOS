#ifndef _EXT2_FS_DRIVER_H_
#define _EXT2_FS_DRIVER_H_

#include "types.h"
#include "managers.h"
#include "../priv_filesystem.h"
#include "ext2_structs.h"

uint32_t _EXT2_Initialize(FileDescriptor *desc);
uint32_t _EXT2_Filesystem_OpenFile(FileDescriptor *desc, const char *filename, int flags, int perms);
uint8_t _EXT2_Filesystem_ReadFile(FileDescriptor *desc, UID id, uint8_t *buffer, size_t size);
uint8_t _EXT2_Filesystem_SeekFile(FileDescriptor *desc, uint32_t fd, uint32_t offset, int whence);
uint8_t _EXT2_Filesystem_CloseFile(FileDescriptor *desc, uint32_t fd);
uint8_t _EXT2_Filesystem_DeleteFile(FileDescriptor *desc, const char *file);
uint8_t _EXT2_Filesystem_RenameFile(FileDescriptor *desc, const char *orig_name, const char *new_name);

uint32_t _EXT2_Filesystem_OpenDir(FileDescriptor *desc, const char *filename);
uint8_t _EXT2_Filesystem_ReadDir(FileDescriptor *desc, uint32_t dd, Filesystem_DirEntry *dir);
uint8_t _EXT2_Filesystem_CloseDir(FileDescriptor *desc, uint32_t fd);
uint8_t _EXT2_Filesystem_MakeDir(FileDescriptor *desc, const char *path);
uint8_t _EXT2_Filesystem_DeleteDir(FileDescriptor *desc, const char *path);

typedef struct EXT2_FD_T
{
    uint32_t id;
    bool is_directory;
    uint32_t inode;
    uint64_t extra_info;
    uint64_t more_extra_info;
    struct EXT2_FD_T *next;
} EXT2_FD;

#endif