#ifndef _PROC_MAN_H_
#define _PROC_MAN_H_

#include "types.h"
#include "processors.h"
#include "managers.h"

#define MAX_PROC_NAME_LEN 128

typedef enum
{
	PROC_PERM_USER = 0,
	PROC_PERM_SUPERUSER = 1,
	PROC_PERM_KERNEL = 3,
	PROC_STATUS_RUNNING = 4
}PROC_FLAGS;

typedef struct ProcessInfo
{
	uint32_t flags;
	UID uid;
	const char name[MAX_PROC_NAME_LEN];
	const char *path;
	struct ProcessInfo *parent;
	struct ProcessInfo *children;
	struct ProcessInfo *next;
}ProcessInfo;

void
ProcessManager_Initialize(void);

UID
ProcessManager_CreateProcess(const char *name,
                             const char *path,
                             uint32_t argc,
                             char **argv,
                             ProcessInfo *parent,
                             uint32_t flags);
uint32_t
ProcessManager_CreateProcess_Syscall(void *param);


UID
ProcessManager_ForkProcess(UID to_fork);

uint32_t
ProcessManager_ForkProcess_Syscall(void *param);

ProcessInfo*
ProcessManager_GetCurProcessInfo(void);

UID
ProcessManager_GetCurPID(void);

uint32_t
ProcessManager_GetCurPID_Syscall(void *param);


#endif