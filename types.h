#ifndef _OS_TYPES_H_
#define _OS_TYPES_H_

#include <stddef.h>
#include <stdint.h>

typedef uint64_t UID;
typedef void (*Callback)(uint32_t);
typedef uint32_t (*Initializer)();
typedef uint8_t bool;

#define TRUE 1
#define FALSE 0

#define ENABLE 1
#define DISABLE 0

#define ENABLED 1
#define DISABLED 0

#endif /* end of include guard: _OS_TYPES_H_ */
