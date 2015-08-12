#ifndef _PRIV_SYS_MAN_H_
#define _PRIV_SYS_MAN_H_

#include "types.h"
#include "system_manager.h"

//Store a single UID and allocating System pair
typedef struct{
  UID uid;
  SysID sys;
}UID_Entry;

typedef struct _UID_LList{
  UID_Entry entries[256];
  struct _UID_LList *next;
}UID_LList;

#endif /* end of include guard: _PRIV_SYS_MAN_H_ */
