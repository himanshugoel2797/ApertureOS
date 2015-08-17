#ifndef _PROCESS_MANAGER_H_
#define _PROCESS_MANAGER_H_

#include "types.h"
#include "managers.h"

struct p_info;

typedef struct t_info {
        UID tid;
        uint32_t esp_phys, ebp_virt, esp_virt;
        uint32_t eip_phys;
        VirtMemMan_Instance page_dir;
        ProcessEntryPoint entryPoint;
        int argc;
        char **argv;
        bool started;
        struct t_info *next;
        struct p_info *parent;
}ThreadInfo;

typedef struct p_info {
        UID pid;
        ThreadInfo *threads;
        struct p_info *next;
}ProcessInfo;

void ProcessMan_Setup();
UID ProcessMan_Create(ProcessEntryPoint entryPoint, int argc, char**argv);
uint32_t ProcessMan_StartProcess(UID id);
uint32_t ProcessMan_StartThread(UID id);
void ProcessMan_ExitDelete(int result);
void ProcessMan_Delete(UID id);


#endif /* end of include guard: _PROCESS_MANAGER_H_ */
