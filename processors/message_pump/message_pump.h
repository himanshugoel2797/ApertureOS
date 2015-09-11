#ifndef _MESSAGE_PUMP_PROC_H_
#define _MESSAGE_PUMP_PROC_H_

#include "types.h"

/**
 * \defgroup msg_pump Message Pump API
 * @{
 */

//! Messages that can be sent by the message pump
typedef enum
{
	MSG_T_NONE = 0,		//!< No message
	MSG_T_ERROR = 1,	//!< Error
	MSG_T_SOCK = 3		//!< Socket message/notification
}PumpMsgTypes;

//! Message Data
typedef struct{
	uint64_t message;	//! The message \sa PumpMsgTypes
	UID source;			//! The source of the message
	void *data;			//! Additional message specific data
}MsgData;

//! Register the current thread to recieve message pump notifications

//! The thread must poll for messages in its queue and act upon them accordingly
void MsgPump_Register();	

//! Send a message through the message pump

//! Send a message to any local application through the message pump
void MsgPump_SendMessage();

/**@}*/

#endif