#ifndef OS2021_API_H
#define OS2021_API_H

#define STACK_SIZE 8192

#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <sys/time.h>
#include <signal.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include "function_libary.h"

typedef struct JsonNode JsonNode;
typedef struct Queue Queue;

struct JsonNode
{
    char *name;
    char *entry;
    char *priority;
    char *C_priority;
    char *state;
    int cancel_mode;
    int PID;
    ucontext_t context;
    long long unsigned Q_Time;
    long long unsigned W_Time;
    int count;
    int Quantum;
    int event_id;
    bool isCancel;
    bool isWaitTime;
    JsonNode *next;
};

struct Queue
{
    JsonNode *Thread;
    Queue *next;
};

struct JsonNode*    HEAD;
struct JsonNode*    temp;
struct Queue**  READY_QUEUE;
struct Queue**  WAIT_QUEUE;
struct Queue*   RUN_QUEUE;
struct Queue*   TREMINATE_QUEUE;
struct itimerval Signaltimer;

ucontext_t  Dispatch_Context,Main;

unsigned init_thread_num;
unsigned PIDindex;
int Priority_Time[3];
bool    inDispatch;

int     OS2021_ThreadCreate(char *job_name, char *p_function, char *priority, int cancel_mode);
void    OS2021_ThreadCancel(char *job_name);
void    OS2021_ThreadWaitEvent(int event_id);
void    OS2021_ThreadSetEvent(int event_id);
void    OS2021_ThreadWaitTime(int msec);
void    OS2021_DeallocateThreadResource();
void    OS2021_TestCancel();

void    CreateContext(ucontext_t *, ucontext_t *, void *);
void    ResetTimer();


void    StartSchedulingSimulation();
/********** Thread Scheduling **********/
void    Dispatcher(void);
void    RUN_QUEUE_Scheduler();

/********** Json Linked List ***********/
void                ReadJson();
struct JsonNode*    JsonNode_Create(char* name,char* entry,char* priority,int cancel_mode, int PID,char *state);
struct JsonNode*    JsonNode_Insert(JsonNode *Head,JsonNode *news);
void                JsonNode_ReadList(JsonNode *Head);
void                JsonNode_Read(JsonNode *Head);
void                JsonNode_Free(JsonNode* head);
/**************** Queue ****************/
int                 Queue_SIZE(Queue *qu);
void                Queue_INIT();
void                Queue_ReadList(Queue *qu);
struct Queue*       Queue_POP(Queue *qu);
struct Queue*       Queue_NODE(JsonNode *now,char * state);
struct Queue*       Queue_PUSH(Queue *qu,JsonNode* now,char* state);
struct JsonNode*    Queue_FRONT(Queue *qu);
void    Start_Scheduling();
/************* Priority *************/
int     Priority_Char2int(char ch);
char*   Priority_int2String(int priority);
/************** signal **************/
void    Signal_Clock_10ms(int sig);
void    Signal_Ctrl_z(int sig);
void    Signal_NULL(int sig);
/************* Debug Mode************/
void    DEBUG_QUEUE();

#endif
