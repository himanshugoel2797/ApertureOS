#ifndef _COM_SERIAL_H_
#define _COM_SERIAL_H_

#include <stddef.h>
#include <stdint.h>

void COM_Initialize();
void COM_WriteStr(const char *str, ...);

#endif /* end of include guard: _COM_SERIAL_H_ */
 
