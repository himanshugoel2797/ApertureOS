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

void strrev(char *str) {
  char temp, *end_ptr;

  /* If str is NULL or empty, do nothing */
  if( str == NULL || !(*str) )
    return;

  end_ptr = str + strlen(str) - 1;

  /* Swap the chars */
  while( end_ptr > str ) {
    temp = *str;
    *str = *end_ptr;
    *end_ptr = temp;
    str++;
    end_ptr--;
  }
}

size_t strlen(const char *str)
{
    size_t size = 0;
    while(str[size] != 0)
    {
      size++;
    }
    return size;
}
