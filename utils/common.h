#ifndef _UTILS_COMMON_H_
#define _UTILS_COMMON_H_

#include <stdint.h>
#include <stddef.h>

#define KB(x) (x * 1024)
#define MB(x) (KB(1) * 1024 * x)
#define GB(x) (uint64_t)(MB(1) * 1024 * x)

#define SET_BIT(x, index) (x | (1 << index))
#define CLEAR_BIT(x, index) (x & ~(1 << index))
#define TOGGLE_BIT(x, index) (x ^ (1 << index))
#define CHECK_BIT(x, index) ((x & (1 << index)) >> index)

void* memcpy(void *dest, void *src, size_t size);
void* memset(void *ptr, int val, size_t num);
void strrev(char *str);
size_t strlen(const char *str);

#endif /* end of include guard: _UTILS_COMMON_H_ */
