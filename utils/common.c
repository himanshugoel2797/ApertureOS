#include "common.h"

void* memcpy(void *dest, void *src, size_t size)
{
        char *d = (char*)dest;
        char *s = (char*)src;

        for(size_t i = 0; i < size; i++)
        {
                d[i] = s[i];
        }
        return dest;
}

void* memset(void *ptr, int val, size_t num)
{
        char* p = (char*)ptr;
        for(size_t i = 0; i < num; i++)
        {
                p[i] = (char)val;
        }
        return ptr;
}
