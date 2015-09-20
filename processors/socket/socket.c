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
    sockets->max_connections = 1;
    sockets->cur_connections = 1;
    sockets->notifications = ~0;
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
    do{
        if(sock->name != NULL && strncmp(sock->name, name, strlen(name)) == 0)
            return SOCK_ERROR_EXISTS;
        sock = sock->next;
    }while(sock != NULL);

    //Check the parameters and make sure they're all valid
    if(desc->flags & SOCK_FEAT_NOTIFICATION)
    {
        sock->notifications = desc->notifications;
    }
    else if(desc->notifications != 0)
        return SOCK_ERROR_UNKNOWN;


    //Register the socket into the table
    sock = kmalloc(sizeof(SocketInfo));
    memset(sock, 0, sizeof(SocketInfo));

    sock->max_connections = desc->max_connections;
    sock->cur_connections = 0;
    sock->connections = NULL;
    sock->lastConnection = NULL;
    sock->flags = desc->flags;

    sock->name = kmalloc(strlen(name) + 1);
    memset(sock->name, 0, strlen(name) + 1);
    strcpy(sock->name, name);

    sock->next = NULL;

    lastSocket->next = sock;
    lastSocket = sock;
    return SOCK_ERROR_NONE;
}

SOCK_ERROR
Socket_Connect(const char *name,
               SocketConnectionDesc *desc)
{
    if(name == NULL | desc == NULL)return SOCK_ERROR_UNKNOWN;
    
    //Find the socket
    SocketInfo *sock = sockets;
    do{
        if(sock->name != NULL && strncmp(sock->name, name, strlen(name)) == 0)
            break;
        sock = sock->next;
    }while(sock != NULL);
    if(sock == NULL)return SOCK_ERROR_NOT_EXIST;

    //Check if connections are available
    if(sock->max_connections != 0 && sock->cur_connections >= sock->max_connections)
        return SOCK_ERROR_NO_FREE_CONNECTIONS;

    //Check if the socket supports the requested features
    if ((sock->flags & desc->flags) != desc->flags)
        return SOCK_ERROR_FEAT_UNAVAILABLE;

    //Ensure this thread doesn't already have an existing open connection
    IntSocketConDesc *e_cons = sock->connections;
    do{
        if(e_cons != NULL && e_cons->tid == ThreadMan_GetCurThreadID())
            return SOCK_ERROR_EXISTS;

        e_cons = e_cons->next;
    }while(e_cons != NULL);


    //Initialize the connections description
    IntSocketConDesc *con = kmalloc(sizeof(IntSocketConDesc));
    memset(con, 0, sizeof(IntSocketConDesc));

    con->flags = desc->flags;
    con->tid = ThreadMan_GetCurThreadID();
    con->clientMessageStream = NULL;
    con->client_queued_message_count = 0;
    con->next = NULL;

    if(sock->lastConnection != NULL)sock->lastConnection->next = con;
    sock->lastConnection = con;
    if(sock->connections == NULL)sock->connections = sock->lastConnection;

    sock->cur_connections++;

    return SOCK_ERROR_NONE; //Connection succeeded
}

SOCK_ERROR
Socket_Disconnect(const char *name)
{
    //Check if a connection to said socket exists from the current thread
    if(name == NULL)return SOCK_ERROR_UNKNOWN;

    SocketInfo *sock = sockets;
    do{
        if(sock->name != NULL && strncmp(sock->name, name, strlen(name)) == 0)
            break;
        sock = sock->next;
    }while(sock != NULL);
    if(sock == NULL)return SOCK_ERROR_NOT_EXIST;


    IntSocketConDesc *e_cons = sock->connections, *prev_con = NULL;
    do{
        if(e_cons != NULL && e_cons->tid == ThreadMan_GetCurThreadID())
            break;
        prev_con = e_cons;
        e_cons = e_cons->next;
    }while(e_cons != NULL);
    if(e_cons == NULL)return SOCK_ERROR_NOT_EXIST;

    //Disconnect
    prev_con->next = e_cons->next;
    sock->cur_connections--;

    //Free the entire message queue
    SocketMessage *msgs = e_cons->clientMessageStream, *next = NULL;
    do{
        if(msgs != NULL)
        {
            next = msgs->next;
            kfree(msgs);
        }
        msgs = next;
    }while(msgs != NULL);

    //Now free the socket connection
    kfree(e_cons);
    return SOCK_ERROR_NONE;
}

SOCK_ERROR
Socket_WriteMessage(const char *name,
                    UID tid,
                    uint32_t cmd,
                    void *params,
                    uint16_t param_size)
{
    if(name == NULL)return SOCK_ERROR_UNKNOWN;

    //Find the relevant socket and connection
    SocketInfo *sock = sockets;
    do{
        if(sock->name != NULL && strncmp(sock->name, name, strlen(name)) == 0)
            break;
        sock = sock->next;
    }while(sock != NULL);
    if(sock == NULL)return SOCK_ERROR_NOT_EXIST;

    IntSocketConDesc *e_cons = sock->connections;
    do{
        if(e_cons != NULL && e_cons->tid == tid)
            break;
        e_cons = e_cons->next;
    }while(e_cons != NULL);
    if(e_cons == NULL)return SOCK_ERROR_NOT_EXIST;

    //Create the message and append it to the message queue


    return SOCK_ERROR_NONE;
}

SOCK_ERROR
Socket_WriteCommand(const char *name,
                    uint32_t cmd,
                    void *params,
                    uint16_t param_size)
{
    if(name == NULL)return SOCK_ERROR_UNKNOWN;

    //Find the relevant socket and connection
    SocketInfo *sock = sockets;
    do{
        if(sock->name != NULL && strncmp(sock->name, name, strlen(name)) == 0)
            break;
        sock = sock->next;
    }while(sock != NULL);
    if(sock == NULL)return SOCK_ERROR_NOT_EXIST;

    //Create the message and append it to the message queue


    return SOCK_ERROR_NONE;
}

SOCK_ERROR
Socket_ReadMessage(const char *name, 
                   uint32_t *cmd, 
                   void *params,
                   uint16_t *param_size)
{
    if(name == NULL)return SOCK_ERROR_UNKNOWN;

    return SOCK_ERROR_NONE;
}