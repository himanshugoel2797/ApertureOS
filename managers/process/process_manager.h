#ifndef _PROCESS_MANAGER_H_
#define _PROCESS_MANAGER_H_

#include "types.h"

typedef struct p_info {
        UID pid;
        uint32_t esp_phys, ebp_phys;
        uint32_t eip;
        uint32_t page_dir;
        struct p_info *next;
}ProcessInfo;

void ProcessMan_Setup();
UID ProcessMan_Create(ProcessEntryPoint entryPoint, int argc, char**argv);


#endif /* end of include guard: _PROCESS_MANAGER_H_ */
