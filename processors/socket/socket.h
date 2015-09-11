#ifndef _SOCKET_PROC_H_
#define _SOCKET_PROC_H_

#include "types.h"
#include "managers.h"

/**
 * \defgroup Sockets Socket API
 * @{
 */

//! A pointer to a function that asynchronously performs a Socket read
typedef uint32_t(*Sock_ReadAsync)(uint8_t* buffer, uint32_t size);

//! A pointer to a function that asynchronously performs a Socket seek
typedef uint32_t(*Sock_SeekAsync)(uint64_t offset, int whence);

//! A pointer to a function that asynchronously performs a Socket write
typedef uint32_t(*Sock_WriteAsync)(uint8_t* src_buf, uint32_t size);

//! Socket errors
typedef enum
{
	SOCK_ERROR_NONE = 0,						//!< No error
	SOCK_ERROR_UNKNOWN = 1 << 0,				//!< Unknown error
	SOCK_ERROR_NO_FREE_CONNECTIONS = 1 << 1,	//!< There are no free connections on this socket
	SOCK_ERROR_NO_PERMS = 1 << 2,				//!< The calling thread doesn't have the permissions
	SOCK_ERROR_FAILED_BUSY = 1 << 3,			//!< The socket is busy
	SOCK_ERROR_EXISTS = 1 << 4,					//!< The socket already exists
	SOCK_ERROR_NOT_EXIST = 1 << 5				//!< The socket does not exist
}SOCK_ERROR;

//! Socket features
typedef enum
{
	SOCK_FEAT_NONE = 0,							//!< No features
	SOCK_FEAT_WRITABLE = 1,						//!< Socket can be written to
	SOCK_FEAT_READABLE = 2,						//!< Socket may be read from
	SOCK_FEAT_SEEKABLE = 4,						//!< Socket is seekable
	SOCK_FEAT_NOTIFICATION = 8,					//!< Socket can send notifications into the message pump
	SOCK_FEAT_SUPERVISOR = 32,					//!< Request supervisor level access
}SOCK_FEATURES;

//! Socket notifications
typedef enum
{
	SOCK_NOTIFICATIONS_NONE = 		0,			//!< Send no notifications
	SOCK_NOTIFICATION_WRITE = 		1<<0,		//!< Send notification on write
	SOCK_NOTIFICATION_READ = 		1<<1,		//!< Send notification on read
	SOCK_NOTIFICATION_SEEK = 		1<<2,		//!< Send notification on seek
	SOCK_NOTIFICATION_CONNECT = 	1<<3,		//!< Send notification on connect
	SOCK_NOTIFICATION_DISCONNECT = 	1<<4,		//!< Send notification on disconnect
	SOCK_NOTIFICATION_COMPLETE = 	1 << 31		//!< Signals the completion of an asynchronous action for the current thread
}SOCK_NOTIFICATIONS;

//! The description of the socket
typedef struct{
	uint32_t size;								//! The size of the struct, must be sizeof(SocketDesc)				
	uint32_t max_connections;					//! The maximum number of connections allowed to the socket

	Sock_ReadAsync read;						//! Function to read from the socket
	Sock_WriteAsync write;						//! Function to write to the socket
	Sock_SeekAsync seek;						//! Function to seek the socket
	
	SOCK_FEATURES flags;						//! Features supported by the socket
	SOCK_NOTIFICATIONS notifications;			//! Notifications send by the socket
}SocketDesc;

//! The description of the socket connection
typedef struct{
	uint32_t size;								//! The size of the struct. must be sizeof(SocketConnectionDesc)
	
	SOCK_FEATURES requested_features;			//! Features requested from the socket
}SocketConnectionDesc;


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
//! \param id 	The UID identifying the connection
//! \return An error code describing the result of the operation
//! \sa SOCK_ERROR, Socket_Disconnect()
SOCK_ERROR 
Socket_Connect(const char *name, 
               SocketConnectionDesc *desc, 
               UID *id);


//! Attempt to disconnect from the socket

//! Note: Disconnection may fail if the socket is busy
//! \param id 	The UID identifying the connection
//! \return An error code describing the result of the operation
//! \sa Socket_Connect(), SOCK_ERROR
SOCK_ERROR 
Socket_Disconnect(UID id);


//! Request an asynchronous read from the socket

//! The socket is required to notify through the message pump with the SOCK_NOTIFICATION_COMPLETE flag set on completion of this request. This operation requires the socket to support reading and the connection to have requested permission to do so
//! \param id 	The UID identifying the connection
//! \param buffer The buffer to write the read data to
//! \param size The size in bytes of data to read
//! \return An error code describing the result of the operation
//! \sa Socket_WriteAsync(), Socket_SeekAsync(), SOCK_NOTIFICATIONS
SOCK_ERROR 
Socket_ReadAsync(UID id, 
                 uint8_t *buffer, 
                 uint32_t size);


//! Request an asynchronous write to the socket

//! The socket is required to notify through the message pump with the SOCK_NOTIFICATION_COMPLETE flag set on completion of this request. This operation requires the socket to support writing and the connection to have requested permission to do so
//! \param id 	The UID identifying the connection
//! \param buffer The buffer to read the data to write from
//! \param size The size in bytes of data to write
//! \return An error code describing the result of the operation
//! \sa Socket_ReadAsync(), Socket_SeekAsync(), SOCK_NOTIFICATIONS
SOCK_ERROR 
Socket_WriteAsync(UID id, 
                  uint8_t *buffer, 
                  uint32_t size);


//! Request an asynchronous seek of the socket

//! The socket is required to notify through the message pump with the SOCK_NOTIFICATION_COMPLETE flag set on completion of this request. This operation requires the socket to support seeking and the connection to have requested permission to do so
//! \param id 	The UID identifying the connection
//! \param offset The offset relative to whence to seek to
//! \param whence The location relative to which to seek
//! \return An error code describing the result of the operation
//! \sa Socket_ReadAsync(), Socket_WriteAsync(), SOCK_NOTIFICATIONS
SOCK_ERROR 
Socket_SeekAsync(UID id, 
                 uint64_t offset, 
                 int whence);

/**@}*/

#endif