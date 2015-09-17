#ifndef _PROC_MAN_H_
#define _PROC_MAN_H_

#include "types.h"
#include "processors.h"
#include "managers.h"

void
ProcessManager_Initialize(void);

UID
ProcessManager_CreateProcess(const char *name,
                             const char *path,
                             uint32_t argc,
                             char **argv);
int
ProcessManager_CreateProcess_SysCall(void *param);
#endif