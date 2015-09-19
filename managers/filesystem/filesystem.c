#include "filesystem.h"
#include "priv_filesystem.h"
#include "utils/common.h"
#include "gpt/gpt.h"
#include "mbr/mbr.h"
#include "ext2/ext2.h"


SystemData *filesys_sys = NULL;
uint32_t filesystem_Initialize();
uint8_t filesystem_messageHandler(Message *msg);

FileDescriptor *descriptors = NULL, *lastDescriptor = NULL;
Filesystem_Driver *fs_drivers = NULL, *lastDriver = NULL;

bool initialized = FALSE;

int retVal = 0;
int GetRetval()
{
    return retVal;
}

void
Filesystem_Setup(void)
{
    filesys_sys = SysMan_RegisterSystem();
    strcpy(filesys_sys->sys_name, "filesystem");

    filesys_sys->prerequisites[0] = 0;     //No prereqs

    filesys_sys->init = filesystem_Initialize;
    filesys_sys->init_cb = NULL;
    filesys_sys->msg_cb = filesystem_messageHandler;

    SysMan_StartSystem(filesys_sys->sys_id);
}

uint32_t
filesystem_Initialize(void)
{
    //Register all filesystem fs_drivers
    fs_drivers = kmalloc(sizeof(Filesystem_Driver));

    fs_drivers->filesystem = EXT2;
    fs_drivers->next = NULL;
    fs_drivers->_H_Initialize = _EXT2_Initialize;
    fs_drivers->_H_Filesystem_OpenFile = _EXT2_Filesystem_OpenFile;
    fs_drivers->_H_Filesystem_ReadFile = _EXT2_Filesystem_ReadFile;
    fs_drivers->_H_Filesystem_SeekFile = _EXT2_Filesystem_SeekFile;
    fs_drivers->_H_Filesystem_CloseFile = _EXT2_Filesystem_CloseFile;
    fs_drivers->_H_Filesystem_DeleteFile = _EXT2_Filesystem_DeleteFile;
    fs_drivers->_H_Filesystem_RenameFile = _EXT2_Filesystem_RenameFile;

    fs_drivers->_H_Filesystem_OpenDir = _EXT2_Filesystem_OpenDir;
    fs_drivers->_H_Filesystem_ReadDir = _EXT2_Filesystem_ReadDir;
    fs_drivers->_H_Filesystem_CloseDir = _EXT2_Filesystem_CloseDir;
    fs_drivers->_H_Filesystem_MakeDir = _EXT2_Filesystem_MakeDir;
    fs_drivers->_H_Filesystem_DeleteDir = _EXT2_Filesystem_DeleteDir;
    fs_drivers->sector_size = 512;  //Default to a sector size of 512 bytes

    lastDriver = fs_drivers;

    retVal = 0xFFFF;
    //Attempt to setup with MBR
    retVal = MBR_Setup(AHCI_0_Read, AHCI_0_Write); 

    if(retVal >= 0)
        goto success_ret;


    //Try again with GPT
    retVal = GPT_Setup(AHCI_0_Read, AHCI_0_Write, &fs_drivers->sector_size);

    if(retVal >= 0)
        goto success_ret;

    //Last attempt, just try directly
    //Setup the first boot disk as the specified type of FS
    retVal = Filesystem_RegisterDescriptor("/", AHCI_0_Read, AHCI_0_Write, BOOT_FS);

    if(retVal  == 0)
        goto success_ret;

    //Fail
    return -1;

    success_ret:
        descriptors = lastDescriptor;
        return retVal;
}

uint8_t
filesystem_messageHandler(Message *msg)
{

}


UID
Filesystem_OpenFile(const char *filename,
                    int flags,
                    int perms)
{
    if(!initialized)return -1;
    //Determine the descriptor based on the filename, call the appropriate function and convert it's returned ID into a UID
    FileDescriptor *desc = (FileDescriptor*)Filesystem_FindDescriptorFromPath(filename);
    graphics_Write("TESTING FILE OPEN!!! %s,%x", 0, 600, filename, desc);
    graphics_SwapBuffer();
    //if(desc == NULL)return -1;
    uint32_t result = desc->driver->_H_Filesystem_OpenFile(desc, filename, flags, perms);
    graphics_Write("TEST2", 0, 620);
    graphics_SwapBuffer();

    return result | (desc->id << 32);
}

uint8_t
Filesystem_ReadFile(UID id,
                    uint8_t *buffer,
                    size_t size)
{
    if(!initialized)return -1;
    FileDescriptor *desc = (FileDescriptor*)Filesystem_FindDescriptorFromUID(id);
    uint8_t result = desc->driver->_H_Filesystem_ReadFile(desc, EXTRACT_ID(id), buffer, size);
    return result;
}

uint8_t
Filesystem_CloseFile(UID fd)
{
    if(!initialized)return -1;
    FileDescriptor *desc = (FileDescriptor*)Filesystem_FindDescriptorFromUID(fd);
    uint8_t result = desc->driver->_H_Filesystem_CloseFile(desc, EXTRACT_ID(fd));
    return result;
}

uint64_t
Filesystem_SeekFile(UID fd,
                    uint64_t offset,
                    int whence)
{
    if(!initialized)return -1;
    FileDescriptor *desc = (FileDescriptor*)Filesystem_FindDescriptorFromUID(fd);
    uint64_t result = desc->driver->_H_Filesystem_SeekFile(desc, EXTRACT_ID(fd), offset, whence);
    return result;
}

uint8_t
Filesystem_DeleteFile(const char *file)
{
    if(!initialized)return -1;
    //Determine the descriptor based on the filename, call the appropriate function and convert it's returned ID into a UID
    FileDescriptor *desc = (FileDescriptor*)Filesystem_FindDescriptorFromPath(file);
    uint8_t result = desc->driver->_H_Filesystem_DeleteFile(desc, file);
    return result;
}

uint8_t
Filesystem_RenameFile(const char *orig_name,
                      const char *new_name)
{
    if(!initialized)return -1;
    //Determine the descriptor based on the filename, call the appropriate function and convert it's returned ID into a UID
    FileDescriptor *desc = (FileDescriptor*)Filesystem_FindDescriptorFromPath(orig_name);
    uint8_t result = desc->driver->_H_Filesystem_RenameFile(desc, orig_name, new_name);
    return result;
}

UID
Filesystem_OpenDir(const char *dirname)
{
    if(!initialized)return -1;
    //Determine the descriptor based on the filename, call the appropriate function and convert it's returned ID into a UID
    FileDescriptor *desc = (FileDescriptor*)Filesystem_FindDescriptorFromPath(dirname);
    uint32_t result = desc->driver->_H_Filesystem_OpenDir(desc, dirname);
    return result | (desc->id << 32);
}


uint8_t
Filesystem_ReadDir(UID dd,
                   Filesystem_DirEntry *dir)
{
    if(!initialized)return -1;
    FileDescriptor *desc = (FileDescriptor*)Filesystem_FindDescriptorFromUID(dd);
    uint8_t result = desc->driver->_H_Filesystem_ReadDir(desc, EXTRACT_ID(dd), dir);
    return result;
}

uint8_t
Filesystem_CloseDir(UID fd)
{
    if(!initialized)return -1;
    FileDescriptor *desc = (FileDescriptor*)Filesystem_FindDescriptorFromUID(fd);
    uint8_t result = desc->driver->_H_Filesystem_CloseDir(desc, EXTRACT_ID(fd));
    return result;
}

uint8_t
Filesystem_MakeDir(const char *path)
{
    if(!initialized)return -1;
    //Determine the descriptor based on the filename, call the appropriate function and convert it's returned ID into a UID
    FileDescriptor *desc = (FileDescriptor*)Filesystem_FindDescriptorFromPath(path);
    uint8_t result = desc->driver->_H_Filesystem_MakeDir(desc, path);
    return result;
}

uint8_t
Filesystem_DeleteDir(const char *path)
{
    if(!initialized)return -1;
    //Determine the descriptor based on the filename, call the appropriate function and convert it's returned ID into a UID
    FileDescriptor *desc = (FileDescriptor*)Filesystem_FindDescriptorFromPath(path);
    uint8_t result = desc->driver->_H_Filesystem_DeleteDir(desc, path);
    return result;
}


uint8_t
Filesystem_Close(UID fd)
{
    if(!initialized)return -1;
    return -1;
}

UID Filesystem_RegisterDescriptor(
    const char *target,
    ReadFunc read,
    WriteFunc write,
    SupportedFilesystems fs)
{
    FileDescriptor *descriptor = descriptors;
    while(descriptor != NULL && descriptor->next != NULL)
        {
            if(strncmp(descriptor->path, target, strlen(target)) == 0)
                {
                    return -1;	//This path has already been hooked
                }
            descriptor = descriptor->next;
        }

    //Find the filesystem driver
    Filesystem_Driver *driver = fs_drivers;
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
    if(descriptors == NULL)descriptors = lastDescriptor;

    graphics_Write("lolo!!!!", 700,700);
    graphics_SwapBuffer();

    desc->path = kmalloc(strlen(target) + 1);
    memset(desc->path, 0, strlen(target) + 1);
    strcpy(desc->path, target);

    desc->read = read;
    desc->write = write;
    desc->data = NULL;
    desc->driver = driver;
    desc->next = NULL;

    desc->id = new_uid();

    uint32_t ret = driver->_H_Initialize(desc); //Tell the filesystem driver to check this device


    if(ret != 0)return -3;

    graphics_Write("Initialized!!!!", 800, 800);
    graphics_SwapBuffer();
    initialized = TRUE;

    return 0;
}

uint8_t
Filesystem_UnregisterDescriptor(UID id)
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
        }
    else
        {
            prev_driver->next = driver->next;
        }

    if(driver->next == NULL)lastDriver = prev_driver;

    if(driver->data != NULL)kfree(driver->data);
    kfree(driver->path);
    kfree(driver);
}

void*
Filesystem_FindDescriptorFromPath(const char *path)
{

    FileDescriptor *descriptor = descriptors;
    uint32_t j = 0;
    while(descriptor != NULL && descriptor->next != NULL)
        {
            graphics_Write(descriptor->path, 500, j);
            j+=30;

            if(strncmp(descriptor->path, path, strlen(descriptor->path)) == 0)
                {
                    return descriptor;	//This path has already been hooked
                }
            descriptor = descriptor->next;
        }

    if(descriptor != NULL){
        graphics_Write(descriptor->path, 600,600);
    }else graphics_Write("NULLOLO", 600,600);
    graphics_SwapBuffer();
    if(descriptor != NULL && strncmp(descriptor->path, path, strlen(descriptor->path)) == 0)
        {
            return descriptor;  //This path has already been hooked
        }

        graphics_Write("NO MATCH", 500, 500);
        graphics_SwapBuffer();

    return NULL;
}

void*
Filesystem_FindDescriptorFromUID(const UID id)
{

    FileDescriptor *descriptor = descriptors;
    UID s_id = id >> 32;
    while(descriptor != NULL && descriptor->next != NULL)
        {
            if( descriptor->id == s_id)
                {
                    return descriptor;	//This path has already been hooked
                }
            descriptor = descriptor->next;
        }

    if( descriptor->id == s_id)
        {
            return descriptor;  //This path has already been hooked
        }
    return NULL;
}

