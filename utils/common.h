#ifndef _UTILS_COMMON_H_
#define _UTILS_COMMON_H_

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

#define KB(x) (x * 1024)
#define MB(x) (KB(1) * 1024 * x)
#define GB(x) (uint64_t)(MB(1) * 1024 * x)

#define SET_BIT(x, index) (x | (1 << index))
#define CLEAR_BIT(x, index) (x & ~(1 << index))
#define TOGGLE_BIT(x, index) (x ^ (1 << index))
#define CHECK_BIT(x, index) ((x & (1 << index)) >> index)
#define SET_VAL_BIT(number, n, x) ( number ^ ((-x ^ number) & (1 << n)))

void* memcpy(void *dest, void *src, size_t size);
void* memset(void *ptr, int val, size_t num);
void strrev(char *str);
size_t strlen(const char *str);
int strncmp(const char * s1, const char * s2, size_t n);
int sprintf ( char * str, const char * format, ... );

#endif /* end of include guard: _UTILS_COMMON_H_ */
