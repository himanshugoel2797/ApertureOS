#ifndef _UTILS_NATIVE_H_
#define _UTILS_NATIVE_H_

#include <stddef.h>
#include <stdint.h>

void outb(const uint16_t port,const uint8_t val);
uint8_t inb(const uint16_t port);

#endif /* end of include guard: _UTILS_NATIVE_H_ */
