#ifndef _PRIV_SOCKET_PROC_H_
#define _PRIV_SOCKET_PROC_H_

#include "types.h"
#include "socket.h"

#define MAX_QUEUED_CLIENT_MESSAGES 32
#define MAX_QUEUED_SERVER_MESSAGES 512

typedef struct SocketMessage{
	uint32_t cmd;
	void *params;
	uint16_t param_size;
	UID tid;
	struct SocketMessage *next;
} SocketMessage;

typedef struct IntSocketConDesc
{
	SOCK_FEATURES flags;	//Requested features
	UID tid;
	SocketMessage *clientMessageStream;
	uint32_t client_queued_message_count;	
	struct IntSocketConDesc *next;
} IntSocketConDesc;

typedef struct SocketInfo
{
    SOCK_FEATURES flags;
    SOCK_NOTIFICATIONS notifications;
    uint32_t max_connections;
    uint32_t cur_connections;
    
	SocketMessage *serverMessageStream;
	uint32_t server_queued_message_count;	
    
	IntSocketConDesc *connections, *lastConnection;

    char *name;
    struct SocketInfo *next;
} SocketInfo;

#endif