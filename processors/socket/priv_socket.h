#ifndef _PRIV_SOCKET_PROC_H_
#define _PRIV_SOCKET_PROC_H_

#include "types.h"
#include "socket.h"

#define MAX_QUEUED_CLIENT_MESSAGES 32
#define MAX_QUEUED_SERVER_MESSAGES 512

typedef struct SocketMessage
{
    uint8_t params[256];
    uint8_t param_size;
    UID tid, pid;
    struct SocketMessage *next;
    struct SocketMessage *prev;
} SocketMessage;

typedef struct SocketInfo
{
    SOCK_FEATURES flags;
    uint32_t max_connections;
    uint32_t cur_connections;

    SocketMessage *messages, *lastMessage;

    char *name;
    struct SocketInfo *next;
} SocketInfo;

typedef struct SocketConnection
{
    SOCK_FEATURES flags;
    SocketInfo *socket;
    struct SocketConnection *next;
} SocketConnection;

#endif