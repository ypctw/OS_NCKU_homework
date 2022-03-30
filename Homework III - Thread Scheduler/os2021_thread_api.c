#include "os2021_thread_api.h"

int OS2021_ThreadCreate(char *job_name, char *p_function, char* priority, int cancel_mode)
{
    JsonNode* t = HEAD;
    while(t)
    {
        if(strcmp(job_name,t->name) == 0)
            return -1;
        t =  t-> next;
    }
    PIDindex++;
    temp = JsonNode_Create(job_name, p_function, priority, cancel_mode, PIDindex, "READY");
    HEAD = JsonNode_Insert(HEAD, temp);
    READY_QUEUE[Priority_Char2int(priority[0])] = Queue_PUSH(READY_QUEUE[Priority_Char2int(priority[0])], temp, "READY");
    if (strcmp(temp->entry, "Function1") == 0)
        CreateContext(&temp->context, NULL, &Function1),printf("[CREATE] Function1 %s\n",temp->name);
    else if (strcmp(temp->entry, "Function2") == 0)
        CreateContext(&temp->context, NULL, &Function2),printf("[CREATE] Function2 %s\n",temp->name);
    else if (strcmp(temp->entry, "Function3") == 0)
        CreateContext(&temp->context, NULL, &Function3),printf("[CREATE] Function3 %s\n",temp->name);
    else if (strcmp(temp->entry, "Function4") == 0)
        CreateContext(&temp->context, NULL, &Function4),printf("[CREATE] Function4 %s\n",temp->name);
    else if (strcmp(temp->entry, "Function5") == 0)
        CreateContext(&temp->context, NULL, &Function5),printf("[CREATE] Function5 %s\n",temp->name);
    else if (strcmp(temp->entry, "ResourceReclaim") == 0)
        CreateContext(&temp->context, NULL, &ResourceReclaim),printf("[CREATE] ResourceReclaim %s\n",temp->name);
    else
        printf("no find\n");
    return PIDindex;
}

void OS2021_ThreadCancel(char *job_name)
{
    printf("[ThreadCancel] %s\n",job_name);
    if(strcmp(RUN_QUEUE->Thread->name,job_name) == 0)
    {
        printf("[Cancel itself]\n");
        if(RUN_QUEUE->Thread->cancel_mode == 0)
        {
            int priority = Priority_Char2int(RUN_QUEUE->Thread->C_priority[0]);
            if(priority >=2)
            {
                printf("The priority of the thread %s is change from %1s to %1s\n",
                       RUN_QUEUE->Thread->name,
                       RUN_QUEUE->Thread->C_priority,
                       Priority_int2String(priority + 1));
                RUN_QUEUE->Thread->C_priority = Priority_int2String(priority + 1);
            }
            TREMINATE_QUEUE = Queue_PUSH(TREMINATE_QUEUE, RUN_QUEUE->Thread, "TERMI");
            swapcontext(&RUN_QUEUE->Thread->context,&Dispatch_Context);
        }
        else
        {
            RUN_QUEUE->Thread->isCancel = true;
        }
    }
    else
    {
        RUN_QUEUE->Thread->Quantum += 200;
        JsonNode *t = HEAD;
        Queue *last,*now;
        while(t != NULL)
        {
            if(strcmp(t->name,job_name) == 0)
            {
                if(t->cancel_mode == 1)
                    t->isCancel = true;
                else
                {
                    if(strcmp(t->state,"WAITING") == 0)
                    {
                        for(int i = 0; i <= 2; i++)
                        {
                            last = WAIT_QUEUE[i], now = WAIT_QUEUE[i];
                            while(now != NULL)
                            {
                                if(strcmp(job_name,now->Thread->name) == 0)
                                {
                                    last->next = now->next;
                                    TREMINATE_QUEUE = Queue_PUSH(TREMINATE_QUEUE,now->Thread,"TERMI");
                                    return;
                                }
                                last = now;
                                now = now->next;
                            }
                        }
                    }
                    else if (strcmp(t->state,"READY") == 0)
                    {
                        for(int i = 0; i <= 2; i++)
                        {
                            last = READY_QUEUE[i],now = READY_QUEUE[i];
                            while(now != NULL)
                            {
                                if(strcmp(job_name,now->Thread->name) == 0)
                                {
                                    last->next = now->next;
                                    TREMINATE_QUEUE = Queue_PUSH(TREMINATE_QUEUE,now->Thread,"TERMI");
                                    return;
                                }
                                last = now;
                                now = now->next;
                            }
                        }
                    }
                }
            }
            t = t->next;
        }
    }
    return;
}

void OS2021_ThreadWaitEvent(int event_id)
{
    printf("%s wants to wait for event %d\n", RUN_QUEUE->Thread->name, event_id);
    RUN_QUEUE->Thread->event_id = event_id;
    int priority = Priority_Char2int(RUN_QUEUE->Thread->C_priority[0]);
    if (priority > 0) // NOT HIGH
    {
        if (RUN_QUEUE->Thread->Quantum > 0)
        {
            RUN_QUEUE->Thread->Quantum += 200;
            printf("The priority of the thread %s is change from %s to %s\n",
                   RUN_QUEUE->Thread->name,
                   RUN_QUEUE->Thread->C_priority,
                   Priority_int2String(priority - 1));
            priority = priority - 1;
            RUN_QUEUE->Thread->C_priority = Priority_int2String(priority);
        }
    }
    RUN_QUEUE->Thread->Quantum = 0;
    WAIT_QUEUE[priority] = Queue_PUSH(WAIT_QUEUE[priority], RUN_QUEUE->Thread, "WAITING");
    printf("[WaitEvent]\t%s\n",RUN_QUEUE->Thread->name);
    swapcontext(&RUN_QUEUE->Thread->context, &Dispatch_Context);
}

void OS2021_ThreadSetEvent(int event_id)
{
    //From Waiting to Ready
    printf("[ThreadSetEvent] event_id %d\n",  event_id);
    Queue *t, *last;
    JsonNode *jt;
    bool find = false;
    for (int i = 0; i < 3; ++i)
    {
        if (WAIT_QUEUE[i] == NULL)
            continue;
        if (WAIT_QUEUE[i]->Thread->event_id == event_id)
        {
            jt = WAIT_QUEUE[i]->Thread;
            WAIT_QUEUE[i] = WAIT_QUEUE[i]->next;
            find = true;
        }
        else // Not Head
        {
            t = WAIT_QUEUE[i], last = WAIT_QUEUE[i];
            while (t != NULL)
            {
                if (t->Thread->event_id == event_id)
                {
                    jt = t->Thread;
                    last->next = t->next; // WAIT_QUEUE
                    find = true;
                    break;
                }
                last = t, t = t->next;
            }
        }
        if (find)
        {
            printf("[ThreadSetEvent] name %s\n",  jt->name);
            printf("%s changes the status of %s to READY.\n", jt->name, RUN_QUEUE->Thread->name);
            int priority = Priority_Char2int(jt->C_priority[0]);
            READY_QUEUE[priority] = Queue_PUSH(READY_QUEUE[priority], jt, "READY");
        }
    }
}

void OS2021_ThreadWaitTime(int msec)
{
    /*
        The running task change its state to WAITING.
        Change the state of the suspended task to READY after 10msec * 10ms.
        Reschedule (schedule next thread to run).
    */
    RUN_QUEUE->Thread->event_id = -1;
    int priority = Priority_Char2int(RUN_QUEUE->Thread->C_priority[0]);
    if (priority > 0) // NOT HIGH
    {
        if (RUN_QUEUE->Thread->Quantum > 0)
        {
            printf("[WaitTime] The priority of the thread %s is change from %1s to %1s\n",
                   RUN_QUEUE->Thread->name,
                   RUN_QUEUE->Thread->C_priority,
                   Priority_int2String(priority - 1));
            RUN_QUEUE->Thread->C_priority = Priority_int2String(priority - 1);
        }
    }
    RUN_QUEUE->Thread->Quantum = (msec-1)*10;
    RUN_QUEUE->Thread->isWaitTime = true;
    WAIT_QUEUE[priority] = Queue_PUSH(WAIT_QUEUE[priority], RUN_QUEUE->Thread, "WAITING");
    inDispatch = true;
    if  (signal(SIGALRM, Signal_NULL) == SIG_ERR)
        printf("SIGALRM ERROR\n"),exit(-1);
    swapcontext(&RUN_QUEUE->Thread->context,&Dispatch_Context);
}

void OS2021_DeallocateThreadResource()
{
    JsonNode *t, *Head,*Head_last;
    while(TREMINATE_QUEUE != NULL)
    {
        t = Queue_FRONT(TREMINATE_QUEUE);
        Head = HEAD;
        if(strcmp(HEAD->name,t->name) == 0)
            HEAD = HEAD->next,JsonNode_Free(Head);
        else
        {
            Head_last = HEAD;
            while(Head !=NULL)
            {
                if(strcmp(Head->name,t->name) == 0)
                {
                    Head_last->next = Head->next,JsonNode_Free(Head);
                    break;
                }
                Head_last = Head;
                Head = Head->next;
            }
        }
        TREMINATE_QUEUE = Queue_POP(TREMINATE_QUEUE);
    }
}
void JsonNode_Free(JsonNode* head)
{
    printf("The Memory space by %s has been released\n",head->name);
    free(head);
    return;
}

void OS2021_TestCancel()
{
    if(!RUN_QUEUE->Thread->isCancel)
        return;
    printf("[TestCancel] name     %s\n",RUN_QUEUE->Thread->name);
    printf("[TestCancel] isCancel %d\n",RUN_QUEUE->Thread->isCancel);
    TREMINATE_QUEUE = Queue_PUSH(TREMINATE_QUEUE, RUN_QUEUE->Thread,"TERMI");
    inDispatch = true;
    if  (signal(SIGALRM, Signal_NULL) == SIG_ERR)
        printf("SIGALRM ERROR\n"),exit(-1);
    swapcontext(&RUN_QUEUE->Thread->context,&Dispatch_Context);
}

void CreateContext(ucontext_t *context, ucontext_t *next_context, void *func)
{
    getcontext(context);
    context->uc_stack.ss_sp = malloc(STACK_SIZE);
    context->uc_stack.ss_size = STACK_SIZE;
    context->uc_stack.ss_flags = 0;
    context->uc_link = next_context;
    makecontext(context, (void (*)(void))func, 0);
}

void ResetTimer()
{
    Signaltimer.it_value.tv_sec = 0;
    Signaltimer.it_value.tv_usec = 10;
    if (setitimer(ITIMER_REAL, &Signaltimer, NULL) < 0)
    {
        printf("ERROR SETTING TIME SIGALRM!\n");
        exit(-1);
    }
}

void Dispatcher(void)
{
    while(1)
    {
        // Maybe need something Cease Timer
        // TODO
        //  RUN QUEUE PUT
        RUN_QUEUE_Scheduler();
        RUN_QUEUE->Thread->Quantum = Priority_Time[Priority_Char2int(RUN_QUEUE->Thread->C_priority[0])];
        inDispatch = false;
        if  (signal(SIGALRM, Signal_Clock_10ms) == SIG_ERR)
            printf("SIGALRM ERROR\n"),exit(-1);
        swapcontext(&Dispatch_Context,&RUN_QUEUE->Thread->context);
    }
}
int Priority_Char2int(char ch)
{
    int priority = 0;
    switch (ch)
    {
    case 'L':
        priority++;
    case 'M':
        priority++;
    case 'H':
        ;
    }
    return priority;
}
char* Priority_int2String(int priority)
{
    if (priority <= 0)
        return "H";
    else if (priority == 1)
        return "M";
    else
        return "L";
}

void StartSchedulingSimulation()
{
    if (signal(SIGALRM, Signal_Clock_10ms) == SIG_ERR)
        printf("SIGALRM ERROR\n"),exit(-1);
    if (signal(SIGTSTP, Signal_Ctrl_z) == SIG_ERR)
        printf("SIGTSTP ERROR\n"),exit(-1);
    Queue_INIT();
    ReadJson();
    //  Queue Initialization and Priority Time
    inDispatch = true;

    Priority_Time[0] = 100, Priority_Time[1] = 200, Priority_Time[2] = 300;
    //  Signal
    Signaltimer.it_interval.tv_usec = 10;
    Signaltimer.it_interval.tv_sec = 0;
    ResetTimer();
    //  Dispatcher
    CreateContext(&Dispatch_Context,NULL,&Dispatcher);
    //  Init_Json Thread
    temp = HEAD->next;
    // Start Scheduling
    inDispatch = true;
    if (signal(SIGALRM, Signal_NULL) == SIG_ERR)
        printf("SIGALRM ERROR\n"),exit(-1);
    setcontext(&Dispatch_Context);
}

void Queue_INIT()
{
    PIDindex = 0;
    // To Initialize All Queue
    READY_QUEUE = (Queue **)malloc(3 * sizeof(Queue *));
    WAIT_QUEUE = (Queue **)malloc(3 * sizeof(Queue *));
    RUN_QUEUE = (Queue *)malloc(sizeof(Queue));
    for (int i = 0; i<3; ++i)
    {
        READY_QUEUE[i] = NULL;
        WAIT_QUEUE[i] = NULL;
    }
    RUN_QUEUE->next = NULL;
    TREMINATE_QUEUE = NULL;
}

void ReadJson()
{
    char buffer[1024], mode[30], name[30], entry[30], priority[2], cancel_mode;
    bool Count_thread = false;
    FILE *fp = fopen("init_threads.json", "r");
    // Create Node
    //JsonNode *Head = JsonNode_Create("reclaimer", "ResourceReclaim", "L", 1, 0, "READY");
    init_thread_num = 1;
    while (fgets(buffer, sizeof(buffer), fp) != NULL)
    {
        sscanf(buffer, "%s", mode);
        if (!Count_thread)
        {
            if (mode[0] == '{')
                Count_thread = true;
        }
        else
        {
            if (mode[0] == '{')
                continue;
            sscanf(buffer, " \"%s\" ", mode);
            switch (mode[0])
            {
            case 'n':
                sscanf(buffer, " \"name\" : \"%2s", name);
                break;
            case 'e':
                sscanf(buffer, " \"entry function\" : \"%9s", entry);
                break;
            case 'p':
                sscanf(buffer, " \"priority\": \"%1s ", priority);
                break;
            case 'c':
                sscanf(buffer, " \"cancel mode\": \"%c ", &cancel_mode);
                //Head = JsonNode_Insert(Head, JsonNode_Create(name, entry, priority, cancel_mode-'0', 0, "READY"));
                OS2021_ThreadCreate(name, entry, priority, cancel_mode - '0');
                init_thread_num += 1;
                break;
            default:
                break;
            }
        }
    }
    fclose(fp);
    OS2021_ThreadCreate("reclaimer", "ResourceReclaim", "L", 1);
    printf("# of init Thread %d\n", init_thread_num);
    return;
}
JsonNode* JsonNode_Create(char *name, char *entry, char* priority, int cancel_mode, int PID, char *state)
{
    JsonNode *news = (JsonNode *)malloc(sizeof(JsonNode));
    news->name = (char *)malloc(20 * sizeof(char));
    news->entry = (char *)malloc(15 * sizeof(char));
    news->priority = (char *)malloc(sizeof(priority));
    news->C_priority = (char *)malloc(sizeof(priority));
    news->priority = priority;
    news->C_priority = priority;
    news->cancel_mode = cancel_mode;
    news->next = NULL;
    news->PID = PID;
    news->state = (char *)malloc(15 * sizeof(char));
    news->W_Time = 0;
    news->Q_Time = 0;
    news->count = 0;
    news->event_id = -1;
    news->isCancel = false;
    news->isWaitTime = false;
    strcpy(news->name, name);
    strcpy(news->entry, entry);
    strcpy(news->state, state);
    return news;
}
JsonNode *JsonNode_Insert(JsonNode *Head, JsonNode *news)
{
    if(Head == NULL)
        return news;
    JsonNode *t = Head;
    while (t->next != NULL)
        t = t->next;
    t->next = news;
    return Head;
}
JsonNode *Queue_FRONT(Queue *qu)
{
    return qu->Thread;
}
Queue *Queue_POP(Queue *qu)
{
    return qu->next;
}
Queue *Queue_NODE(JsonNode *now, char *state)
{
    Queue *t = (Queue *)malloc(sizeof(Queue));
    t->next = NULL;
    t->Thread = now;
    t->Thread->state = (char *)malloc(sizeof(state));
    t->Thread->state = state;
    return t;
}
Queue *Queue_PUSH(Queue *qu, JsonNode *now, char *state)
{
    if (qu == NULL)
        return Queue_NODE(now, state);
    Queue *t = qu;
    while (t->next != NULL)
        t = t->next;
    t->next = Queue_NODE(now, state);
    return qu;
}
int Queue_SIZE(Queue *qu)
{
    Queue *t = qu;
    int result = 0;
    while (t != NULL)
        ++result, t = t->next;
    return result;
}
void Queue_ReadList(Queue *qu)
{
    Queue *t = qu;
    while (t != NULL)
        JsonNode_Read(t->Thread), t = t->next;
}
void RUN_QUEUE_Scheduler()
{
    for (int i = 0; i <= 2; ++i)
    {
        if (READY_QUEUE[i] != NULL)
        {
            RUN_QUEUE->Thread = Queue_FRONT(READY_QUEUE[i]);
            RUN_QUEUE->Thread->state = (char *)malloc(10 * sizeof(char));
            strcpy(RUN_QUEUE->Thread->state, "RUNNING");
            READY_QUEUE[i] = Queue_POP(READY_QUEUE[i]);
            return;
        }
    }
    return;
}
void Signal_Ctrl_z(int sig)
{
    JsonNode *report = HEAD;
    printf("\n*****************************************************************************************\n");
    printf("*\tTID\tName\t\tState\tB_Priority\tC_Priority\tQ_Time\tW_Time\t*\n");
    while (report != NULL)
    {
        printf("*\t%d\t", report->PID);
        if (strlen(report->name) > 3)
            printf("%s\t", report->name);
        else
            printf("%s\t\t", report->name);
        printf("%s\t", report->state);
        printf("%1s\t\t", report->priority);
        printf("%1s\t\t", report->C_priority);
        printf("%llu\t", report->Q_Time);
        printf("%llu\t*\n", report->W_Time);
        report = report->next;
    }
    printf("*****************************************************************************************\n");
}
void Signal_NULL(int sig)
{
    return;
}
void Signal_Clock_10ms(int sig)
{
    if(inDispatch)
        return;
    Queue *t,*t_last;
    //  WAIT Queue , READY Queue Timer Count
    for (int i = 0; i <= 2; ++i)
    {
        t = READY_QUEUE[i];
        while (t != NULL)
            t->Thread->Q_Time += 10, t = t->next;
        t = WAIT_QUEUE[i];
        while (t != NULL)
        {
            t->Thread->W_Time += 10;
            if(t->Thread->isWaitTime)
                t->Thread->Quantum -= 10;
            t = t->next;
        }
    }
    //  WAIT Queue Quantum
    for (int i = 0; i <= 2; ++i)
    {
        if(WAIT_QUEUE[i] == NULL)
            continue;
        if(WAIT_QUEUE[i]->Thread->isWaitTime)
        {
            if(WAIT_QUEUE[i]->Thread->Quantum < 0)
            {
                WAIT_QUEUE[i]->Thread->isWaitTime = false;
                READY_QUEUE[i] = Queue_PUSH(READY_QUEUE[i],WAIT_QUEUE[i]->Thread,"READY");
                WAIT_QUEUE[i] = WAIT_QUEUE[i]->next;
                break;
            }
        }
        else
        {
            t = WAIT_QUEUE[i], t_last = WAIT_QUEUE[i];
            if(t->Thread->isWaitTime)
            {
                if(t->Thread->Quantum < 0)
                {
                    //printf("[DEBUG Wait time up] %s\n",t->Thread->name);
                    t_last->next = t->next;
                    READY_QUEUE[i] = Queue_PUSH(READY_QUEUE[i],t->Thread,"READY");
                }
                t_last = t,t = t->next;
            }
        }
    }
    // RUN Quantum
    if(RUN_QUEUE != NULL)
        RUN_QUEUE->Thread->Quantum -= 1;
    if (RUN_QUEUE->Thread->Quantum < 0)
    {
        RUN_QUEUE->Thread->Quantum = 0;
        // Context Switch
        int priority = Priority_Char2int(RUN_QUEUE->Thread->C_priority[0]);
        if (priority < 2 && RUN_QUEUE->Thread->isCancel == false) // NOT LOW
        {
            printf("The priority of the thread %s is change from %1s to %1s\n",
                   RUN_QUEUE->Thread->name,
                   RUN_QUEUE->Thread->C_priority,
                   Priority_int2String(priority + 1));
            priority += 1;
        }
        RUN_QUEUE->Thread->C_priority = Priority_int2String(priority);
        READY_QUEUE[priority] = Queue_PUSH(READY_QUEUE[priority], RUN_QUEUE->Thread, "READY");
        inDispatch = true;
        if  (signal(SIGALRM, Signal_NULL) == SIG_ERR)
            printf("SIGALRM ERROR\n"),exit(-1);
        swapcontext(&RUN_QUEUE->Thread->context,&Dispatch_Context);
    }
}
void JsonNode_ReadList(JsonNode *Head)
{
    while (Head != NULL)
        JsonNode_Read(Head), Head = Head->next;
}
void JsonNode_Read(JsonNode *Head)
{
    printf("%9s %15s %1s %2d %d %s\n",
           Head->name, Head->entry, Head->priority, Head->cancel_mode, Head->PID, Head->state);
}
void DEBUG_QUEUE()
{
    printf("\n*************************************************\n");

    Queue *tt;
    for (int i = 0; i <= 2; ++i)
    {
        tt = WAIT_QUEUE[i];
        while (tt != NULL)
            printf("Wait[%d] = %s\n", i, tt->Thread->name), tt = tt->next;
    }
    for (int i = 0; i <= 2; ++i)
    {
        tt = READY_QUEUE[i];
        while (tt != NULL)
            printf("Ready[%d] = %s\n", i, tt->Thread->name), tt = tt->next;
    }
    printf("RUNNING -> %s\n",RUN_QUEUE->Thread->name);
    tt = TREMINATE_QUEUE;
    printf("TERMI ");
    while (tt != NULL)
        printf("-> %s", tt->Thread->name), tt = tt->next;
    printf("\n");
    JsonNode_ReadList(HEAD);
    printf("\n*************************************************\n");

}

