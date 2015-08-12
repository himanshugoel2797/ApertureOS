#ifndef _OS_TYPES_H_
#define _OS_TYPES_H_

#include <stddef.h>
#include <stdint.h>

typedef uint64_t UID;
typedef void (*Callback)(uint32_t);
typedef uint32_t (*Initializer)();

#endif /* end of include guard: _OS_TYPES_H_ */
