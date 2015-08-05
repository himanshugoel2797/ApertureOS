#ifndef _UTILS_COMMON_H_
#define _UTILS_COMMON_H_

#include <stdint.h>
#include <stddef.h>

void* memcpy(void *dest, void *src, size_t size);
void* memset(void *ptr, int val, size_t num);
void strrev(char *str);
size_t strlen(const char *str);

#endif /* end of include guard: _UTILS_COMMON_H_ */
