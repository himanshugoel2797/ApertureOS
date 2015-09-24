#include "socket.h"
#include "priv_socket.h"

SocketInfo *sockets = NULL, *lastSocket = NULL;

uint32_t
Socket_Initialize(void)
{
    sockets = kmalloc(sizeof(SocketInfo));
    memset(sockets, 0, sizeof(SocketInfo));
    lastSocket = sockets;

    sockets->name = NULL;
    sockets->max_connections = 0;
    sockets->cur_connections = 0;
    sockets->flags = ~0;
    sockets->next = NULL;

    return 0;
}

SOCK_ERROR
Socket_Create(const char *name,
              SocketDesc *desc)
{
    if(desc == NULL | name == NULL)return SOCK_ERROR_UNKNOWN;

    //Make sure a socket by this name doesn't already exist
    SocketInfo *sock = sockets;
    do
        {
            if(sock->name != NULL && strncmp(sock->name, name, strlen(name)) == 0)
                return SOCK_ERROR_EXISTS;
            sock = sock->next;
        }
    while(sock != NULL);


    //Register the socket into the table
    sock = kmalloc(sizeof(SocketInfo));
    memset(sock, 0, sizeof(SocketInfo));

    sock->max_connections = desc->max_connections;
    sock->cur_connections = 0;
    sock->flags = desc->flags;

    sock->name = kmalloc(strlen(name) + 1);
    memset(sock->name, 0, strlen(name) + 1);
    strcpy(sock->name, name);

    sock->next = NULL;

    lastSocket->next = sock;
    lastSocket = sock;

    //Connect the current thread to the socket
    SocketConnectionDesc connection;
    connection.size = sizeof(SocketConnectionDesc);
    connection.flags = desc->flags;
    Socket_Connect(name, &connection);

    return SOCK_ERROR_NONE;
}

SOCK_ERROR
Socket_Connect(const char *name,
               SocketConnectionDesc *desc)
{
    if(name == NULL | desc == NULL)return SOCK_ERROR_UNKNOWN;

    //Find the socket
    SocketInfo *sock = sockets;
    do
        {
            if(sock->name != NULL && strncmp(sock->name, name, strlen(name)) == 0)
                break;
            sock = sock->next;
        }
    while(sock != NULL);
    if(sock == NULL)return SOCK_ERROR_NOT_EXIST;

    //Check if connections are available
    if(sock->max_connections != 0 && sock->cur_connections >= sock->max_connections)
        return SOCK_ERROR_NO_FREE_CONNECTIONS;

    //Check if the socket supports the requested features
    if ((sock->flags & desc->flags) != desc->flags)
        return SOCK_ERROR_FEAT_UNAVAILABLE;


    SocketConnection *cons = (SocketConnection*)kmalloc(sizeof(SocketConnection));
    cons->socket = sock;
    cons->flags = desc->flags;
    cons->next = NULL;

    if(ThreadMan_GetCurThreadTLS()->sock_info == NULL)ThreadMan_GetCurThreadTLS()->sock_info = cons;
    else
        {
            uint32_t *addr = (uint32_t*)((SocketConnection*)ThreadMan_GetCurThreadTLS()->sock_info)->next;
            ((SocketConnection*)ThreadMan_GetCurThreadTLS()->sock_info)->next = cons;
            cons->next = addr;
        }

    sock->cur_connections++;

    return SOCK_ERROR_NONE; //Connection succeeded
}

SOCK_ERROR
Socket_Disconnect(const char *name)
{
    //Check if a connection to said socket exists from the current thread
    if(name == NULL)return SOCK_ERROR_UNKNOWN;

    SocketInfo *sock = sockets, *prev_sock = NULL;
    do
        {
            if(sock->name != NULL && strncmp(sock->name, name, strlen(name)) == 0)
                break;
            prev_sock = sock;
            sock = sock->next;
        }
    while(sock != NULL);
    if(sock == NULL)return SOCK_ERROR_NOT_EXIST;


    //Find the connection info in the socket
    SocketConnection *cons = (SocketConnection*)ThreadMan_GetCurThreadTLS()->sock_info, *prev = NULL;
    do
        {
            if(strncmp(name, cons->socket->name, strlen(name)) == 0)break;
            prev = cons;
            cons = cons->next;
        }
    while(cons != NULL);
    if(cons == NULL)return SOCK_ERROR_NOT_EXIST;

    //Remove the connection
    prev->next = cons->next;
    kfree(cons);

    if(sock->cur_connections == 0)
        {
            //TODO free all messages from list

            prev_sock->next = sock->next;   //Remove the socket from the list
            kfree(sock->name);
            kfree(sock);
        }

    return SOCK_ERROR_NONE;
}

SOCK_ERROR
Socket_WriteMessage(const char *name,
                    UID tid,
                    void *params,
                    uint8_t param_size)
{
    if(name == NULL)return SOCK_ERROR_UNKNOWN;

    //Find the relevant socket and connection
    //Find the connection info in the socket
    SocketConnection *cons = (SocketConnection*)ThreadMan_GetCurThreadTLS()->sock_info;
    do
        {
            if(strncmp(name, cons->socket->name, strlen(name)) == 0)break;
            cons = cons->next;
        }
    while(cons != NULL);
    if(cons == NULL)return SOCK_ERROR_NOT_EXIST;

    SocketInfo *sock = cons->socket;

    //Create the message and append it to the message queue
    SocketMessage *msg = (SocketMessage*)kmalloc(sizeof(SocketMessage));
    memcpy(msg->params, params, (uint32_t)param_size + 1);

    msg->param_size = param_size;
    msg->tid = ThreadMan_GetCurThreadID();
    msg->pid = ProcessManager_GetCurPID();
    msg->next = NULL;
    msg->prev = NULL;

    if(sock->messages == NULL)sock->messages = sock->lastMessage = msg;
    else
        {
            sock->lastMessage->next = msg;
            msg->prev = sock->lastMessage;
            sock->lastMessage = msg;
        }

    return SOCK_ERROR_NONE;
}

SOCK_ERROR
Socket_ReadMessage(const char *name,
                   UID *src_pid,
                   UID *src_tid,
                   void *params,
                   uint8_t *param_size)
{
    if(name == NULL | params == NULL | param_size == NULL)return SOCK_ERROR_UNKNOWN;

    //Find the relevant socket and connection
    //Find the connection info in the socket
    SocketConnection *cons = (SocketConnection*)ThreadMan_GetCurThreadTLS()->sock_info;
    do
        {
            if(strncmp(name, cons->socket->name, strlen(name)) == 0)break;
            cons = cons->next;
        }
    while(cons != NULL);
    if(cons == NULL)return SOCK_ERROR_NOT_EXIST;

    SocketInfo *sock = cons->socket;


    //Fetch the next message from the list
    SocketMessage *msg = sock->lastMessage;

    sock->lastMessage = msg->prev;
    sock->lastMessage->next = NULL;

    memcpy(params, msg->params, (uint32_t)msg->param_size + 1);
    *param_size = msg->param_size;

    kfree(msg);

    return SOCK_ERROR_NONE;
}