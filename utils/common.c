#include "common.h"

/*
 * Copyright (C) 1991,1992,1993,1997,1998,2003, 2005 Free Software Foundation, Inc.
 * This file is part of the GNU C Library.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
/* From glibc-2.14, sysdeps/i386/memcpy.c */
void* memcpy(void *dest, void *src, size_t size)
{
    unsigned long d0, d1, d2;
    asm volatile (
        "rep ; movsl\n\t"
        "movl %4,%%ecx\n\t"
        "rep ; movsb\n\t"
        : "=&c" (d0), "=&D" (d1), "=&S" (d2)
        : "0" (size >> 2), "g" (size & 3), "1" (dest), "2" (src)
        : "memory"
    );
    return dest;
}

/*
 * Copyright (C) 1991,1992,1993,1997,1998,2003, 2005 Free Software Foundation, Inc.
 * This file is part of the GNU C Library.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
/* From glibc-2.14, sysdeps/i386/memset.c */
void* memset(void *dstpp, int c, size_t len)
{
    int d0;
    unsigned long int dstp = (unsigned long int) dstpp;
    /* This explicit register allocation improves code very much indeed. */
    register uint32_t x asm ("ax");
    x = (unsigned char) c;
    /* Clear the direction flag, so filling will move forward.  */
    asm volatile ("cld");
    /* This threshold value is optimal.  */
    if (len >= 12)
    {
        /* Fill X with four copies of the char we want to fill with. */
        x |= (x << 8);
        x |= (x << 16);
        /* Adjust LEN for the bytes handled in the first loop.  */
        len -= (-dstp) % sizeof(uint32_t);
        /*
         * There are at least some bytes to set. No need to test for
         * LEN == 0 in this alignment loop.
         */
        /* Fill bytes until DSTP is aligned on a longword boundary. */
        asm volatile (
            "rep\n"
            "stosb" /* %0, %2, %3 */ :
            "=D" (dstp), "=c" (d0) :
            "0" (dstp), "1" ((-dstp) % sizeof(uint32_t)), "a" (x) :
            "memory");
        /* Fill longwords.  */
        asm volatile (
            "rep\n"
            "stosl" /* %0, %2, %3 */ :
            "=D" (dstp), "=c" (d0) :
            "0" (dstp), "1" (len / sizeof(uint32_t)), "a" (x) :
            "memory");
        len %= sizeof(uint32_t);
    }
    /* Write the last few bytes. */
    asm volatile (
        "rep\n"
        "stosb" /* %0, %2, %3 */ :
        "=D" (dstp), "=c" (d0) :
        "0" (dstp), "1" (len), "a" (x) :
        "memory");
    return dstpp;
}

void strrev(char *str)
{
    char temp, *end_ptr;

    /* If str is NULL or empty, do nothing */
    if( str == NULL || !(*str) )
        return;

    end_ptr = str + strlen(str) - 1;

    /* Swap the chars */
    while( end_ptr > str )
    {
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

int strncmp(const char* s1, const char* s2, size_t n)
{
    while(n--)
        if(*s1++!=*s2++)
            return *(unsigned char*)(s1 - 1) - *(unsigned char*)(s2 - 1);
    return 0;
}

char * strcpy ( char * destination, const char * source )
{
    return (char*)memcpy(destination, (void*)source, strlen(source));
}

char *strchr(const char *s, int c)
{
    while (*s != (char)c)
        if (!*s++)
            return 0;
    return (char *)s;
}

char *strrchr(const char *s, int c)
{
    char *e = s + strlen(s);
    while (*e != (char)c)
        if (e-- == s)
            return 0;
    return (char *)e;
}

UID uids_base = 0;
UID new_uid()
{
    return (++uids_base & 0xFFFFFFFF);
}