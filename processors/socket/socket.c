#include "socket.h"
#include "priv_socket.h"

SocketInfo *sockets = NULL, *lastSocket = NULL;
IntSocketConDesc *con_descs = NULL, *con_lastDesc = NULL;

uint32_t
Socket_Initialize(void)
{
    sockets = kmalloc(sizeof(SocketInfo));
    memset(sockets, 0, sizeof(SocketInfo));
    lastSocket = sockets;

    sockets->name = NULL;
    sockets->max_connections = 1;
    sockets->cur_connections = 1;
    sockets->notifications = ~0;
    sockets->flags = ~0;
    sockets->next = NULL;


    con_descs = kmalloc(sizeof(IntSocketConDesc));
    memset(con_descs, 0, sizeof(IntSocketConDesc));
    con_lastDesc = con_descs;

    con_descs->flags = ~0;
    con_descs->tid = ThreadMan_GetCurThreadID();
    con_descs->next = NULL;

    return 0;
}

SOCK_ERROR
Socket_Create(const char *name,
              SocketDesc *desc)
{
    if(desc == NULL | name == NULL)return SOCK_ERROR_UNKNOWN;

    //Make sure a socket by this name doesn't already exist
    SocketInfo *sock = sockets;
    do{
        if(sock->name != NULL && strncmp(sock->name, name, strlen(name)) == 0)
            return SOCK_ERROR_EXISTS;
        sock = sock->next;
    }while(sock != NULL);

    //Register the socket into the table
    sock = kmalloc(sizeof(SocketInfo));
    memset(sock, 0, sizeof(SocketInfo));

    sock->max_connections = desc->max_connections;
    sock->cur_connections = 0;
    sock->flags = desc->flags;

    if(desc->flags & SOCK_FEAT_NOTIFICATION)
    {
        sock->notifications = desc->notifications;
    }
    else if(desc->notifications != 0)
        goto fail_and_exit;

    sock->name = kmalloc(strlen(name) + 1);
    memset(sock->name, 0, strlen(name) + 1);
    strcpy(sock->name, name);

    sock->next = NULL;

    lastSocket->next = sock;
    lastSocket = sock;
    return SOCK_ERROR_NONE;

    fail_and_exit:
        kfree(sock);
        return SOCK_ERROR_UNKNOWN;
}

SOCK_ERROR
Socket_Connect(const char *name,
               SocketConnectionDesc *desc,
               UID *id)
{
    //Find the socket

    //Check if connections are available

    //Connect if possible
}

SOCK_ERROR
Socket_Disconnect(UID id)
{
    //Check if the current thread is connected to said socket

    //Attempt to disconnect
}

SOCK_ERROR
Socket_WriteCommand(UID id,
                    uint32_t cmd,
                    void *params)
{

}

SOCK_ERROR
Socket_ReadMessage(UID id, 
                   uint32_t *cmd, 
                   void **params)
{

}