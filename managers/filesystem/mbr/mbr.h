#ifndef _MBR_H_
#define _MBR_H_

#include "types.h"
#include "../filesystem.h"

//* Search the provided disk for an MBR partition table, if found, mount all recognized partitions

//* /param read The function to read from the disk
//* /param write The function to write to the disk
//* /return <0 if MBR not found
int32_t
MBR_Setup(ReadFunc read,
          WriteFunc write);

#endif