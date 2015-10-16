#ifndef _MANAGERS_H_
#define _MANAGERS_H_

#include "msg_manager/msg_types.h"

#include "mouse/mouse.h"						   //Mouse Manager
#include "filesystem/filesystem.h"				   //Filesystem manager
#include "keyboard/keyboard.h"                     //Keyboard Manager
#include "interrupt/interrupt_manager.h"           //Interrupt Manager
#include "threads/threads.h"					   //Threading library
#include "timer/timer_manager.h"                   //Timer Manager
#include "phys_mem_manager/phys_mem_manager.h"     //Physical memory manager
#include "virt_mem_manager/virt_mem_manager.h"     //Virtual memory manager
#include "bootstrap_mem_pool/bootstrap_mem_pool.h" //Bootstrap memory pool
#include "msg_manager/msg_manager.h"               //System Error Manaager
#include "system_manager/system_manager.h"         //System Manager
#include "device_manager/device_manager.h"		   //Device Manager

#endif /* end of include guard: _MANAGERS_H_ */
