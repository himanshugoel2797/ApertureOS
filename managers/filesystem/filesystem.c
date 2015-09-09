#include "filesystem.h"
#include "priv_filesystem.h"
#include "utils/common.h"

#include "ext2/ext2.h"


SystemData *filesys_sys = NULL;
uint32_t filesystem_Initialize();
uint8_t filesystem_messageHandler(Message *msg);

FileDescriptor *descriptors = NULL, *lastDescriptor = NULL;
Filesystem_Driver *drivers = NULL, *lastDriver = NULL;

UID fd_base = 0;

void Filesystem_Setup()
{
    filesys_sys = SysMan_RegisterSystem();
    strcpy(filesys_sys->sys_name, "filesystem");

    filesys_sys->prerequisites[0] = 0;     //No prereqs

    filesys_sys->init = filesystem_Initialize;
    filesys_sys->init_cb = NULL;
    filesys_sys->msg_cb = filesystem_messageHandler;

    SysMan_StartSystem(filesys_sys->sys_id);
}

uint32_t filesystem_Initialize()
{

    //Register all filesystem drivers
    drivers = kmalloc(sizeof(Filesystem_Driver));
    drivers->filesystem = EXT2;
    drivers->next = NULL;
    drivers->_H_Initialize = _EXT2_Initialize;
    drivers->_H_Filesystem_OpenFile = _EXT2_Filesystem_OpenFile;
    drivers->_H_Filesystem_ReadFile = _EXT2_Filesystem_ReadFile;
    drivers->_H_Filesystem_SeekFile = _EXT2_Filesystem_SeekFile;
    drivers->_H_Filesystem_CloseFile = _EXT2_Filesystem_CloseFile;
    drivers->_H_Filesystem_DeleteFile = _EXT2_Filesystem_DeleteFile;
    drivers->_H_Filesystem_RenameFile = _EXT2_Filesystem_RenameFile;

    drivers->_H_Filesystem_OpenDir = _EXT2_Filesystem_OpenDir;
    drivers->_H_Filesystem_ReadDir = _EXT2_Filesystem_ReadDir;
    drivers->_H_Filesystem_CloseDir = _EXT2_Filesystem_CloseDir;
    drivers->_H_Filesystem_MakeDir = _EXT2_Filesystem_MakeDir;
    drivers->_H_Filesystem_DeleteDir = _EXT2_Filesystem_DeleteDir;

    lastDriver = drivers;



    //Setup the first boot disk as the specified type of FS
    Filesystem_RegisterDescriptor("/", AHCI_Read, NULL, BOOT_FS);
    descriptors = lastDescriptor;
}

uint8_t filesystem_messageHandler(Message *msg)
{

}


UID Filesystem_OpenFile(const char *filename, int flags, int perms)
{
    //Determine the descriptor based on the filename, call the appropriate function and convert it's returned ID into a UID
    FileDescriptor *desc = (FileDescriptor*)Filesystem_FindDescriptorFromPath(filename);
    uint32_t result = desc->driver->_H_Filesystem_OpenFile(desc, filename, flags, perms);

    if(result != 0)return result + desc->fd_base;
    return -1;
}

uint8_t Filesystem_ReadFile(UID id, uint8_t *buffer, size_t size)
{
    FileDescriptor *desc = (FileDescriptor*)Filesystem_FindDescriptorFromUID(id);
    uint8_t result = desc->driver->_H_Filesystem_ReadFile(desc, id - desc->fd_base, buffer, size);
    return result;
}

uint8_t Filesystem_CloseFile(UID fd)
{
    FileDescriptor *desc = (FileDescriptor*)Filesystem_FindDescriptorFromUID(fd);
    uint8_t result = desc->driver->_H_Filesystem_CloseFile(desc, fd);
    return result;
}

uint8_t Filesystem_SeekFile(UID fd, uint32_t offset, int whence)
{
    FileDescriptor *desc = (FileDescriptor*)Filesystem_FindDescriptorFromUID(fd);
    uint8_t result = desc->driver->_H_Filesystem_SeekFile(desc, fd, offset, whence);
    return result;
}

uint8_t Filesystem_DeleteFile(const char *file)
{
    //Determine the descriptor based on the filename, call the appropriate function and convert it's returned ID into a UID
    FileDescriptor *desc = (FileDescriptor*)Filesystem_FindDescriptorFromPath(file);
    uint8_t result = desc->driver->_H_Filesystem_DeleteFile(desc, file);
    return result;
}

uint8_t Filesystem_RenameFile(const char *orig_name, const char *new_name)
{
    //Determine the descriptor based on the filename, call the appropriate function and convert it's returned ID into a UID
    FileDescriptor *desc = (FileDescriptor*)Filesystem_FindDescriptorFromPath(orig_name);
    uint8_t result = desc->driver->_H_Filesystem_RenameFile(desc, orig_name, new_name);
    return result;
}

UID Filesystem_OpenDir(const char *dirname)
{
    //Determine the descriptor based on the filename, call the appropriate function and convert it's returned ID into a UID
    FileDescriptor *desc = (FileDescriptor*)Filesystem_FindDescriptorFromPath(dirname);
    uint32_t result = desc->driver->_H_Filesystem_OpenDir(desc, dirname);

    if(result != 0)return result + desc->dir_base;
    return -1;
}


uint8_t Filesystem_ReadDir(UID dd, Filesystem_DirEntry *dir)
{
    FileDescriptor *desc = (FileDescriptor*)Filesystem_FindDescriptorFromUID(dd);
    uint8_t result = desc->driver->_H_Filesystem_ReadDir(desc, dd - desc->dir_base, dir);
    return result;
}

uint8_t Filesystem_CloseDir(UID fd)
{
    FileDescriptor *desc = (FileDescriptor*)Filesystem_FindDescriptorFromUID(fd);
    uint8_t result = desc->driver->_H_Filesystem_CloseDir(desc, fd);
    return result;
}

uint8_t Filesystem_MakeDir(const char *path)
{
    //Determine the descriptor based on the filename, call the appropriate function and convert it's returned ID into a UID
    FileDescriptor *desc = (FileDescriptor*)Filesystem_FindDescriptorFromPath(path);
    uint8_t result = desc->driver->_H_Filesystem_MakeDir(desc, path);
    return result;
}

uint8_t Filesystem_DeleteDir(const char *path)
{
    //Determine the descriptor based on the filename, call the appropriate function and convert it's returned ID into a UID
    FileDescriptor *desc = (FileDescriptor*)Filesystem_FindDescriptorFromPath(path);
    uint8_t result = desc->driver->_H_Filesystem_DeleteDir(desc, path);
    return result;
}


uint8_t Filesystem_Close(UID fd)
{
    return -1;
}

UID Filesystem_RegisterDescriptor(
    const char *target,
    ReadFunc *read,
    WriteFunc *write,
    SupportedFilesystems fs)
{
    FileDescriptor *descriptor = descriptors;
    while(descriptor != NULL && descriptor->next != NULL)
    {
        if(strncmp(descriptor->path, target, strlen(target)) == 0) {
            return -1;	//This path has already been hooked
        }
        descriptor = descriptor->next;
    }

    //Find the filesystem driver
    Filesystem_Driver *driver = drivers;
    while(driver != NULL && driver->next != NULL)
    {
        if(driver->filesystem == fs)break;
        driver = driver->next;
    }
    if(driver->next == NULL && driver->filesystem != fs)return -2;	//Invalid FS

    //Add this entry to the last descriptor
    FileDescriptor *desc = kmalloc(sizeof(FileDescriptor));
    if(lastDescriptor != NULL)lastDescriptor->next = desc;
    lastDescriptor = desc;

    desc->path = kmalloc(strlen(target) + 1);
    strcpy(desc->path, target);

    desc->read = read;
    desc->write = write;
    desc->data = NULL;
    desc->driver = driver;
    desc->next = NULL;

    desc->fd_base = fd_base;
    fd_base += MAX_OPEN_FILES;
    desc->dir_base = fd_base;
    fd_base += MAX_OPEN_DIRS;

    desc->id = fd_base++;

    uint32_t ret = driver->_H_Initialize(desc);	//Tell the filesystem driver to check this device
    if(ret != 0)return -3;

    return desc->id;
}

uint8_t Filesystem_UnregisterDescriptor(UID id)
{
    FileDescriptor *driver = descriptors, *prev_driver = NULL;
    while(driver != NULL && driver->next != NULL)
    {
        if(driver->id == id)break;
        prev_driver = driver;
        driver = driver->next;
    }
    if(driver->next == NULL && driver->id != id)return -2;	//Invalid FS

    if(prev_driver == NULL)
    {
        descriptors = driver->next;
    } else
    {
        prev_driver->next = driver->next;
    }

    if(driver->next == NULL)lastDriver = prev_driver;

    if(driver->data != NULL)kfree(driver->data);
    kfree(driver->path);
    kfree(driver);
}

void* Filesystem_FindDescriptorFromPath(const char *path)
{

    FileDescriptor *descriptor = descriptors;
    while(descriptor != NULL && descriptor->next != NULL)
    {
        if(strncmp(descriptor->path, path, strlen(descriptor->path)) == 0) {
            return descriptor;	//This path has already been hooked
        }
        descriptor = descriptor->next;
    }

    if(descriptor != NULL && strncmp(descriptor->path, path, strlen(descriptor->path)) == 0) {
        return descriptor;	//This path has already been hooked
    }

    return NULL;
}

void* Filesystem_FindDescriptorFromUID(const UID id)
{

    FileDescriptor *descriptor = descriptors;
    while(descriptor != NULL && descriptor->next != NULL)
    {
        if( descriptor->fd_base <= id && (descriptor->dir_base + MAX_OPEN_DIRS) >= id) {
            return descriptor;	//This path has already been hooked
        }
        descriptor = descriptor->next;
    }
    
    if( descriptor->fd_base <= id && (descriptor->dir_base + MAX_OPEN_DIRS) >= id) {
        return descriptor;  //This path has already been hooked
    }
    return NULL;
}

