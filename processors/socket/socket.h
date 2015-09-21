#ifndef _SOCKET_PROC_H_
#define _SOCKET_PROC_H_

#include "types.h"
#include "managers.h"

/**
 * \defgroup Sockets Socket API
 * @{
 */


//! Socket errors
typedef enum
{
    SOCK_ERROR_NONE = 0,						//!< No error
    SOCK_ERROR_UNKNOWN = 1 << 0,				//!< Unknown error
    SOCK_ERROR_NO_FREE_CONNECTIONS = 1 << 1,	//!< There are no free connections on this socket
    SOCK_ERROR_NO_PERMS = 1 << 2,				//!< The calling thread doesn't have the permissions
    SOCK_ERROR_FAILED_BUSY = 1 << 3,            //!< The socket is busy
    SOCK_ERROR_EXISTS = 1 << 4,                 //!< The socket or connection already exists
    SOCK_ERROR_NOT_EXIST = 1 << 5,              //!< The socket does not exist
    SOCK_ERROR_FEAT_UNAVAILABLE = 1 << 6        //!< One of the requested features isn't available
} SOCK_ERROR;

//! Socket features
typedef enum
{
    SOCK_FEAT_NONE = 0,							//!< No features
    SOCK_FEAT_NOTIFICATION = 8,					//!< Socket can send notifications into the message pump
    SOCK_FEAT_SUPERVISOR = 32,					//!< Request supervisor level access to the socket
} SOCK_FEATURES;

//! Socket notifications
typedef enum
{
    SOCK_NOTIFICATIONS_NONE = 		0,			//!< Send no notifications
    SOCK_NOTIFICATION_CONNECT = 	1<<1,		//!< Send notification on connect
    SOCK_NOTIFICATION_DISCONNECT = 	1<<2,		//!< Send notification on disconnect
    SOCK_NOTIFICATION_COMPLETE = 	1 << 31		//!< Signals the completion of an asynchronous action for the current thread
} SOCK_NOTIFICATIONS;

//! The description of the socket
typedef struct
{
    uint32_t size;								//! The size of the struct, must be sizeof(SocketDesc)
    uint32_t max_connections;					//! The maximum number of connections allowed to the socket

    SOCK_FEATURES flags;						//! Features supported by the socket
    SOCK_NOTIFICATIONS notifications;			//! Notifications send by the socket
} SocketDesc;

//! The description of the socket connection
typedef struct
{
    uint32_t size;								//! The size of the struct. must be sizeof(SocketConnectionDesc)

    SOCK_FEATURES flags;			//! Features requested from the socket
} SocketConnectionDesc;

uint32_t
Socket_Initialize(void);

//! Create a new Socket and register it to the kernel

//! \param name The name of the socket
//! \param desc The description of the socket
//! \return An error code describing the result of the operation
//! \sa SOCK_ERROR
SOCK_ERROR
Socket_Create(const char *name,
              SocketDesc *desc);


//! Attempt to connect to a socket

//! \param name The name of the socket to connect to
//! \param desc The description of the requested connection
//! \return An error code describing the result of the operation
//! \sa SOCK_ERROR, Socket_Disconnect()
SOCK_ERROR
Socket_Connect(const char *name,
               SocketConnectionDesc *desc);


//! Attempt to disconnect from the socket connection on the current thread

//! \param name The name of the socket
//! \return An error code describing the result of the operation
//! \sa Socket_Connect(), SOCK_ERROR
SOCK_ERROR
Socket_Disconnect(const char *name);


//! Send a command to the socket

//! The socket is required to notify through the message pump with the SOCK_NOTIFICATION_COMPLETE flag set on completion of this request.

//! \param name The name of the socket
//! \param cmd The command to send
//! \param params The command specific parameters to send
//! \param param_size The size of the parameters data received
//! \return An error code describing the result of the operation
//! \sa Socket_Connect(), Socket_ReadMessage(), SOCK_NOTIFICATIONS
SOCK_ERROR
Socket_WriteCommand(const char *name,
                    uint32_t cmd,
                    void *params,
                    uint16_t param_size);


//! Send a message to the client socket from the server

//! Send a message to the client socket from the server

//! \param name The name of the socket
//! \param tid The thread ID of the connection to send the message to, '0' to send to all
//! \param cmd The command to send
//! \param params The command specific parameters to send
//! \param param_size The size of the parameters data sent
//! \return An error code describing the result of the operation
//! \sa Socket_Connect(), Socket_ReadMessage(), SOCK_NOTIFICATIONS
SOCK_ERROR
Socket_WriteMessage(const char *name,
                    UID tid,
                    uint32_t cmd,
                    void *params,
                    uint16_t param_size);


//! Poll the socket for pending messages

//! The socket communicates through messages, the socket server polls its message queue for commands, the socket clients poll their message queues for notifications

//! \param name The name of the socket
//! \param cmd The command received
//! \param params The command specific parameters received, this must be a 64KiB buffer to be able to store all the parameters
//! \param param_size The size of the parameters data received
//! \return An error code describing the result of the operation
//! \sa Socket_Connect(), Socket_WriteCommand()
SOCK_ERROR
Socket_ReadMessage(const char *name,
                   uint32_t *cmd,
                   void *params,
                   uint16_t *param_size);

/**@}*/

#endif