#ifndef _FILESYSTEM_MANAGER_H_
#define _FILESYSTEM_MANAGER_H_

#include "types.h"
#include "managers.h"
#include "drivers.h"
#include "kmalloc.h"

//Pointer to function to read from the hard drive
typedef uint8_t(*ReadFunc)(uint64_t, uint32_t, uint16_t*);

//Pointer to function to write to the hard drive
typedef uint8_t(*WriteFunc)(uint64_t, uint32_t, uint16_t*);

typedef enum
{
    NONE = 0,
    EXT2 = 1
} SupportedFilesystems;

typedef enum
{
    O_RDONLY = 0,
    O_RD_WR = 1,
    O_APPED = 2,
    O_CREAT = 4
} FILE_IO_FLAGS;

typedef enum
{
    SEEK_SET = 0,
    SEEK_CUR = 1,
    SEEK_END = 2
} FILE_IO_WHENCE;

typedef struct
{
    char dir_name[256];
    uint8_t name_len;
    uint8_t type;
} Filesystem_DirEntry;

void Filesystem_Setup();

UID Filesystem_OpenFile(const char *filename, int flags, int perms);
uint8_t Filesystem_CloseFile(UID fd);
uint64_t Filesystem_SeekFile(UID fd, uint64_t offset, int whence);
uint8_t Filesystem_DeleteFile(const char *file);
uint8_t Filesystem_RenameFile(const char *orig_name, const char *new_name);

uint64_t
Filesystem_ReadFile(UID id,
                    uint8_t *buffer,
                    size_t size);

UID Filesystem_OpenDir(const char *filename);
uint8_t Filesystem_ReadDir(UID dd, Filesystem_DirEntry *dir);
uint8_t Filesystem_CloseDir(UID fd);
uint8_t Filesystem_MakeDir(const char *path);
uint8_t Filesystem_DeleteDir(const char *path);

void* Filesystem_FindDescriptorFromPath(const char *path);
void* Filesystem_FindDescriptorFromUID(const UID id);

uint8_t Filesystem_Close(UID fd);

UID Filesystem_RegisterDescriptor(
    const char *target,
    ReadFunc read,
    WriteFunc write,
    SupportedFilesystems fs);

uint8_t Filesystem_UnregisterDescriptor(UID id);

#endif