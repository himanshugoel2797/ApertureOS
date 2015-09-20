#ifndef _K_TERM_PROC_H_
#define _K_TERM_PROC_H_

#include "types.h"
#include "managers.h"

uint32_t
Terminal_Start(void);

void
Terminal_Write(char *str,
               size_t len);

void
Terminal_KeyboardThread(int argc,
                        char **argv);

void
Terminal_DisplayThread(void);

#endif