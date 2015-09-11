#include "socket.h"
#include "priv_socket.h"

SocketInfo *sockets = NULL, *lastSocket = NULL;

SOCK_ERROR 
Socket_Create(const char *name, 
              SocketDesc *desc)
{
	//Make sure a socket by this name doesn't already exist

	//Register the socket into the table

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
Socket_ReadAsync(UID id, 
                 uint8_t *buffer, 
                 uint32_t size)
{
	//Check if this connection is valid

	//Check if this connection has the necessary permissions

	//Queue an asynchronous read from the socket, sending a notification when done
}

SOCK_ERROR 
Socket_WriteAsync(UID id, 
                  uint8_t *buffer, 
                  uint32_t size)
{
	//Check if this connection is valid

	//Check if this connection has the necessary permissions

	//Queue an asynchronous write from the socket, sending a notification when done
}

SOCK_ERROR 
Socket_SeekAsync(UID id, 
                 uint64_t offset, 
                 int whence)
{
	//Check if this connection is valid

	//Check if this connection has the necessary permissions

	//Queue an asynchronous seek from the socket, sending a notification when done
}