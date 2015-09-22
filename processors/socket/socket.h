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
    SOCK_ERROR_NOT_EXIST = 1 << 5,              //!< The socket or connection does not exist
    SOCK_ERROR_FEAT_UNAVAILABLE = 1 << 6        //!< One of the requested features isn't available
} SOCK_ERROR;

//! Socket features
typedef enum
{
    SOCK_FEAT_NONE = 0,							//!< No features
    SOCK_FEAT_SUPERVISOR = 32,					//!< Request supervisor level access to the socket
} SOCK_FEATURES;

//! The description of the socket
typedef struct
{
    uint32_t size;								//! The size of the struct, must be sizeof(SocketDesc)
    uint32_t max_connections;					//! The maximum number of connections allowed to the socket

    SOCK_FEATURES flags;						//! Features supported by the socket
} SocketDesc;

//! The description of the socket connection
typedef struct
{
    uint32_t size;								//! The size of the struct. must be sizeof(SocketConnectionDesc)

    SOCK_FEATURES flags;			             //! Features requested from the socket
} SocketConnectionDesc;

uint32_t
Socket_Initialize(void);

//! Create a new Socket and register it to the kernel, automatically connect the calling thread to the socket, this counts towards the connection count, sockets are automatically deleted when they have no connections

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


//! Send a message to the client socket from the server

//! Send a message to the client socket from the server

//! \param name The name of the socket
//! \param tid The thread ID of the connection to send the message to, '0' to send to all
//! \param params The command specific parameters to send
//! \param param_size The size of the parameters data sent (zero based, ie. 0 = 1 byte)
//! \return An error code describing the result of the operation
//! \sa Socket_Connect(), Socket_ReadMessage(), SOCK_NOTIFICATIONS
SOCK_ERROR
Socket_WriteMessage(const char *name,
                    UID tid,
                    void *msg,
                    uint8_t msg_len);


//! Poll the socket for pending messages

//! The socket communicates through messages, the socket server polls its message queue for commands, the socket clients poll their message queues for notifications

//! \param name The name of the socket
//! \param params The command specific parameters received, this must be a 256 byte buffer to be able to store all the parameters
//! \param src_pid The process ID of the source
//! \param src_tid The thread ID of the source
//! \param param_size The size of the parameters data received (zero based, ie. 0 = 1 byte)
//! \return An error code describing the result of the operation
//! \sa Socket_Connect(), Socket_WriteCommand()
SOCK_ERROR
Socket_ReadMessage(const char *name,
                   UID *src_pid,
                   UID *src_tid,
                   void *msg,
                   uint8_t *msg_len);

/**@}*/

#endif