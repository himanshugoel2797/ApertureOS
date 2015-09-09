#ifndef _MSG_TYPES_H_
#define _MSG_TYPES_H_

#include "types.h"

#define MESSAGE_LEN 128

typedef enum
{
    MI_NONE = 0,
    MI_INITIALIZATION_FAILURE = 1,
    MI_UNSUPPORTED_FEATURE = 2,
    MI_OUT_OF_MEMORY = 3,
} MESSAGE_ID;

typedef enum
{
    MT_NOTIFICATION = 0,
    MT_WARNING = 1,
    MT_ERROR = 2,
} MESSAGE_TYPE;

typedef enum
{
    MP_NORMAL = 0,
    MP_CRITICAL = 1
} MESSAGE_PRIORITY;

typedef struct
{
    char message[MESSAGE_LEN];
    UID system_id;
    UID src_id;
    uint8_t *stack_pointer; //This is used to generate a stack trace (ebp)
    MESSAGE_ID msg_id;
    MESSAGE_TYPE msg_type;
    MESSAGE_PRIORITY msg_priority;
} Message;

typedef uint8_t (*MsgCallback)(Message*);

#endif /* end of include guard: _MSG_TYPES_H_ */
