#ifndef _GPT_FS_MAN_DRIV_SCAN_H_
#define _GPT_FS_MAN_DRIV_SCAN_H_

#include "types.h"
#include "../filesystem.h"

//* Search the provided disk for a GUID partition table, if found, mount all recognized partitions

//* /param read The function to read from the disk
//* /param write The function to write to the disk
//* /return 0 if GPT not found, else number of partitions found
int32_t
GPT_Setup(ReadFunc read,
          WriteFunc write,
          uint32_t *sector_size);


bool
GPT_0_Write(uint64_t start,
            uint32_t count,
            uint16_t *buf);

bool
GPT_0_Read(uint64_t start,
           uint32_t count,
           uint16_t *buf);

#endif