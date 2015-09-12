#ifndef _UTILS_COMMON_H_
#define _UTILS_COMMON_H_

#include "types.h"
#include <stdarg.h>

#define SET_BIT(x, index) (x | (1 << index))
#define CLEAR_BIT(x, index) (x & ~(1 << index))
#define TOGGLE_BIT(x, index) (x ^ (1 << index))
#define CHECK_BIT(x, index) ((x & (1 << index)) >> index)
#define SET_VAL_BIT(number, n, x) ( number ^ ((-x ^ number) & (1 << n)))

void* memcpy(void *dest, void *src, size_t size);
void* memset(void *ptr, int val, size_t num);
void strrev(char *str);
size_t strlen(const char *str);
char* strcpy ( char * destination, const char * source );
int strncmp(const char * s1, const char * s2, size_t n);
int vsnprintf ( char * str, const char * format, va_list vl );
int sprintf ( char * str, const char * format, ... );
char* utoa(uint64_t val, char *ostr, int base);
char* itoa(int64_t val, char *ostr, int base);
char* strchr(const char *s, int c);
char* strrchr(const char *s, int c);

UID new_uid();

#endif /* end of include guard: _UTILS_COMMON_H_ */
