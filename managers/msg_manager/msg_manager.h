#ifndef _MESSAGE_MANAGER_H_
#define _MESSAGE_MANAGER_H_

#include <stddef.h>
#include <stdint.h>

#define ERROR_MESSAGE_LEN 128
#define ERROR_POOL_SIZE 512

typedef enum {
        MI_INITIALIZATION_FAILURE = 0,
        MI_UNSUPPORTED_FEATURE = 1
} MESSAGE_ID;

typedef enum {
        MT_NOTIFICATION = 0,
        MT_WARNING = 1,
        MT_ERROR = 2,
}MESSAGE_TYPE;

typedef enum {
        MP_NORMAL = 0,
        MP_CRITICAL = 1
}MESSAGE_PRIORITY;

typedef struct {
        char message[ERROR_MESSAGE_LEN];
        uint32_t system_id;
        uint8_t *stack_pointer; //This is used to generate a stack trace (ebp)
        MESSAGE_ID msg_id;
        MESSAGE_TYPE msg_type;
        MESSAGE_PRIORITY msg_priority;
}Message;

#endif /* end of include guard: _MESSAGE_MANAGER_H_ */
