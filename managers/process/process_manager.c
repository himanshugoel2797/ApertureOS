#include "process_manager.h"
#include "managers.h"
#include "drivers.h"
#include "utils/common.h"

SystemData *pman_sys = NULL;
uint32_t processMan_Initialize();
void processMan_callback(uint32_t res);
uint8_t processMan_messageHandler(Message *msg);

UID id = 1;
ProcessInfo processes[128]; //Allocate 128 initial process objects
ThreadInfo threads[512];    //Allocate 512 initial thread objects

ProcessInfo *rootProcess, *curProc;
ThreadInfo *rootThread, *curThread;

void ProcessMan_Setup()
{
        /*
           Use the APIC Timer to setup an interrupt every few microseconds, in the interrupt handler we switch the esp and ebp
         */

        pman_sys = SysMan_RegisterSystem();
        strcpy(pman_sys->sys_name, "processMan");
        pman_sys->prerequisites[0] = 0;              //No prereqs
        pman_sys->init = processMan_Initialize;
        pman_sys->init_cb = processMan_callback;
        pman_sys->msg_cb = processMan_messageHandler;

        SysMan_StartSystem(pman_sys->sys_id);
}


uint32_t processMan_Initialize()
{
        memset(processes, 0, sizeof(processes));
        memset(threads, 0, sizeof(threads));
        id = 1;
        curProc = rootProcess = NULL;
        curThread = rootThread = NULL;
        return 0;
}

void processMan_callback(uint32_t res)
{

}

uint8_t processMan_messageHandler(Message *msg)
{

}

UID ProcessMan_Create(ProcessEntryPoint entryPoint, int argc, char**argv)
{
        for(int i = 0; i < 128; i++)
        {
                if(processes[i].pid == 0)
                {
                        processes[i].pid = id++;
                        for(int j = 0; j < 512; j++)
                        {
                                if(threads[j].tid == 0)
                                {
                                        threads[j].tid = id++;
                                        threads[j].entryPoint = entryPoint;
                                        threads[j].next = NULL;
                                        threads[j].argc = argc;
                                        threads[j].argv = argv;
                                        threads[j].started = FALSE;
                                        threads[j].parent = &processes[i];
                                        processes[i].threads = &threads[j];

                                        if(curThread != NULL) curThread->next = &threads[j];
                                        if(rootThread == NULL) rootThread = &threads[j];
                                        curThread = &threads[i];
                                        curThread->next = NULL;

                                        //TODO allocate thread specific stack space

                                }
                        }
                        if(curProc != NULL) curProc->next = &processes[i];
                        if(rootProcess == NULL) rootProcess = &processes[i];
                        curProc = &processes[i];
                        curProc->next = NULL;
                        return curProc->pid;
                }
        }
}

uint32_t ProcessMan_StartProcess(UID id)
{
        //Find the process
        ProcessInfo *proc = rootProcess;
        while(proc->next != NULL)
        {
                if(proc->pid == id)
                {
                        return ProcessMan_StartThread(proc->threads->tid);
                }
                proc = proc->next;
        }
        return -1;
}

uint32_t ProcessMan_StartThread(UID id)
{
        ThreadInfo *thrd = rootThread;
        while(thrd->next != NULL)
        {
                if(thrd->tid == id)
                {
                        //Setup the stack and set eip
                        thrd->started = TRUE;
                }
        }
}
