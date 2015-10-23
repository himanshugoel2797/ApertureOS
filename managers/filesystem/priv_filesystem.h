#ifndef _PRIV_FILESYSTEM_MANAGER_H_
#define _PRIV_FILESYSTEM_MANAGER_H_

#include "types.h"
#include "managers.h"
#include "drivers.h"

#include "filesystem.h"

#define MAX_OPEN_FILES 0x10000000
#define MAX_OPEN_DIRS 0x10000000

#define EXTRACT_ID(n) (n & 0x0000FFFF)

typedef struct FileDescriptor_T FileDescriptor;

typedef struct Filesystem_Driver_T
{
    uint32_t(*_H_Initialize)(FileDescriptor *desc);
    uint32_t(*_H_Filesystem_OpenFile)(FileDescriptor *desc, const char *filename, int flags, int perms);
    uint64_t(*_H_Filesystem_ReadFile)(FileDescriptor *desc, UID id, uint8_t *buffer, size_t size);
    uint64_t(*_H_Filesystem_SeekFile)(FileDescriptor *desc, uint32_t fd, uint64_t offset, int whence);
    uint8_t(*_H_Filesystem_CloseFile)(FileDescriptor *desc, uint32_t fd);
    uint8_t(*_H_Filesystem_DeleteFile)(FileDescriptor *desc, const char *file);
    uint8_t(*_H_Filesystem_RenameFile)(FileDescriptor *desc, const char *orig_name, const char *new_name);

    uint32_t(*_H_Filesystem_OpenDir)(FileDescriptor *desc, const char *filename);
    uint8_t(*_H_Filesystem_ReadDir)(FileDescriptor *desc, uint32_t dd, Filesystem_DirEntry *dir);
    uint8_t(*_H_Filesystem_CloseDir)(FileDescriptor *desc, uint32_t fd);
    uint8_t(*_H_Filesystem_MakeDir)(FileDescriptor *desc, const char *path);
    uint8_t(*_H_Filesystem_DeleteDir)(FileDescriptor *desc, const char *path);

    uint32_t sector_size;
    SupportedFilesystems filesystem;
    struct Filesystem_Driver_T *next;
} Filesystem_Driver;

struct FileDescriptor_T
{
    char *path;
    ReadFunc read;
    WriteFunc write;
    Filesystem_Driver *driver;
    void *data;
    UID id;
    FileDescriptor *next;
};

#endif