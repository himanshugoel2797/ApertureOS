#ifndef _MESSAGE_PUMP_PROC_H_
#define _MESSAGE_PUMP_PROC_H_

#include "types.h"

typedef enum
{
	MSG_T_NONE = 0,
	MSG_T_ERROR = 1,
	MSG_T_SOCK = 3
}PumpMsgTypes;

typedef struct{
	uint64_t message;
	UID source;
	void *data;
}

void MsgPump_Register();	//Register the current thread to recieve message pump notifications
void MsgPump_SendMessage()

#endif