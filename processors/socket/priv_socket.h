#ifndef _PRIV_SOCKET_PROC_H_
#define _PRIV_SOCKET_PROC_H_

#include "types.h"
#include "socket.h"

typedef struct
{
    Sock_ReadAsync read;
    Sock_WriteAsync write;
    Sock_SeekAsync seek;
    SOCK_FEATURES flags;
    SOCK_NOTIFICATIONS notifications;
    uint32_t max_connections;
    uint32_t cur_connections;
    char *name;
} SocketInfo;


#endif