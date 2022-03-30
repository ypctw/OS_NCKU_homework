#include "MemManager.h"

void Initialize()
{
    TLB_Clean();
    Initialize_Config();
    PAGETABLE_Create_Table(PROCESSES, VIRTUAL_PAGE);
    PHYSICAL_MEMORY_Create_Table(PHYSICAL_FRAME);
    DISK = 0;
    DISK_Kick = -1;
    Global_clock_pointer = 0;
    Local_clock_pointer = (int *)malloc(sizeof(int) * PROCESSES);
    TLBHit = (double *)malloc(sizeof(double) * PROCESSES);
    LookUp = (double *)malloc(sizeof(double) * PROCESSES);
    PageFault = (double *)malloc(sizeof(double) * PROCESSES);
    Reference = (double *)malloc(sizeof(double) * PROCESSES);
    for (int i = 0; i < PROCESSES; ++i)
        Local_clock_pointer[i] = 0, TLBHit[i] = 0.0, LookUp[i] = 0.0, PageFault[i] = 0.0, Reference[i] = 0.0;
}
void Clean()
{
    free(TLBHit);
    free(LookUp);
    free(PageFault);
    free(PHYSICAL_MEMORY);
    for (int i = 0; i < PROCESSES; ++i)
        free(PAGETABLE[i]);
    free(PAGETABLE);
    while (Pqueue != NULL)
        Pnode_Pop();
}
void DEBUG()
{
    fprintf(fPTB, "-------------------------------------\n");
    for (int i = 0; i < TLB_NUM; i++)
        fprintf(fTLB, "%2d | %3d %3d %3d\n", i, TLB[i].vpn, TLB[i].pfn, TLB[i].lru);

    for (int i = 0; i < VIRTUAL_PAGE; ++i)
    {
        for (int j = 0; j < PROCESSES; ++j)
            fprintf(fPTB, "%3d |%3d %3d|   ||  ", i, PAGETABLE[j][i].present, PAGETABLE[j][i].pfn_or_dbi);
        fprintf(fPTB, "\n");
    }
    for (int i = 0; i < PHYSICAL_FRAME; ++i)
    {
        fprintf(fPM, "%3d| %c %2d %3d", i, PHYSICAL_MEMORY[i].process_int + 'A', PHYSICAL_MEMORY[i].clock, PHYSICAL_MEMORY[i].vpn);
        if (Global_clock_pointer == i)
            fprintf(fPM, " <-- Global");
        fprintf(fPM, "\n");
    }
    fprintf(fPTB, "-------------------------------------\n");
    Pnode *temp = Pqueue;
    while (temp != NULL)
    {
        printf("%d ", temp->pFrame);
        temp = temp->next;
    }
    printf("\n");
}
void Analysis()
{
    FILE *fA = fopen("analysis.txt", "w");
    for (int i = 0; i < PROCESSES; ++i)
    {
        double hit_ratio = TLBHit[i] / LookUp[i];
        double page_fault_rate = PageFault[i] / Reference[i];
        double EAT = hit_ratio * (100 + 20) + (1 - hit_ratio) * (2 * 100 + 20);
        // printf("Page Fault %f %f\n", PageFault[i], Reference[i]);
        fprintf(fA, "Process %c, Effective Access Time = %.3f\n", i + 'A', EAT);
        fprintf(fA, "Process %c, Page Fault Rate = %.3f\n", i + 'A', page_fault_rate);
    }
    fclose(fA);
}
int main()
{
    srand(time(NULL));
    Initialize();
    // Init TLB
    FILE *fR = fopen("trace.txt", "r");
    FILE *fW = fopen("trace_output.txt", "w");
    //fTLB = fopen("./Record/TLB.txt", "w");
    //fPTB = fopen("./Record/PageTable.txt", "w");
    //fPM = fopen("./Record/Physical.txt", "w");
    char Process_C;
    char Last_Process_c = 'a';
    int VPN;
    while (fscanf(fR, "Reference(%c, %d)\n", &Process_C, &VPN) != EOF)
    {
        Reference[Process_C - 'A']++;
        // fprintf(fW, "Reference(%c, %3d)", Process_C, VPN);
        /*{
            printf("Reference(%c, %3d)\n", Process_C, VPN);
            fprintf(fPM, "Reference(%c, %3d)\n", Process_C, VPN);
            fprintf(fTLB, "Reference(%c, %3d)\n", Process_C, VPN);
            fprintf(fPTB, "Reference(%c, %3d)\n", Process_C, VPN);
        }*/
        if (Last_Process_c - Process_C != 0) // Clean TLB
            TLB_Clean();
        // TLB Check
        int TLB_hit = TLB_check(VPN);
        LookUp[Process_C - 'A']++;
        if (TLB_hit != -1) // TLB hit
        {
            TLB[TLB_hit].lru = TLB_lru++;
            int pfn = TLB[TLB_hit].pfn;
            PHYSICAL_MEMORY[pfn].clock = 1;
            fprintf(fW, "Process %c, TLB Hit, %d=>%d\n", Process_C, VPN, pfn);

            // TLB Hit
            TLBHit[Process_C - 'A']++;
        }
        else // TLB miss
        {
            int PageTable_hit = PAGETABLE_check(Process_C - 'A', VPN);
            if (PageTable_hit == -1) // TLB miss Page Fault not in Disk
            {
                if (!PHYSICAL_empty_check()) // Physical Memory is Not FULL
                {
                    // Put in PAGETABLE
                    PAGETABLE[Process_C - 'A'][VPN].present = 1;
                    PAGETABLE[Process_C - 'A'][VPN].pfn_or_dbi = Global_clock_pointer;
                    // PUT in TLB
                    TLB_Policy(VPN, Global_clock_pointer, Process_C - 'A');
                    // Physical Memory
                    PHYSICAL_MEMORY[Global_clock_pointer].clock = 1;
                    PHYSICAL_MEMORY[Global_clock_pointer].process_int = Process_C - 'A';
                    PHYSICAL_MEMORY[Global_clock_pointer].vpn = VPN;
                    // To FIFO queue
                    Pqueue = Pnode_Push(Global_clock_pointer);
                    // To Output
                    fprintf(fW, "Process %c, TLB Miss, Page Fault, %d,", Process_C, PAGETABLE[Process_C - 'A'][VPN].pfn_or_dbi);
                    fprintf(fW, " Evict -1 of Process %c to -1, %d<<-1\n", Process_C, VPN);

                    fprintf(fW, "Process %c, TLB Hit, %d=>%d\n", Process_C, VPN, Global_clock_pointer);

                    //  For Analysis : LookUp Again
                    LookUp[Process_C - 'A']++;
                    TLBHit[Process_C - 'A']++;
                    PageFault[Process_C - 'A']++;

                    Global_clock_pointer = (Global_clock_pointer + 1) % PHYSICAL_FRAME;
                    Local_clock_pointer[Process_C - 'A'] = Global_clock_pointer;
                }
                else // Physical Memory is FULL
                {
                    int pfn = frame_swap(Process_C - 'A');
                    int swap_out_vpn = PHYSICAL_MEMORY[pfn].vpn;
                    int swap_out_process_int = PHYSICAL_MEMORY[pfn].process_int;
                    // Put in PageTable
                    PAGETABLE[Process_C - 'A'][VPN].pfn_or_dbi = pfn;
                    PAGETABLE[Process_C - 'A'][VPN].present = 1;

                    PAGETABLE[swap_out_process_int][swap_out_vpn].present = 0;
                    if (DISK_Kick == -1)
                        PAGETABLE[swap_out_process_int][swap_out_vpn].pfn_or_dbi = DISK++;
                    else
                        PAGETABLE[swap_out_process_int][swap_out_vpn].pfn_or_dbi = DISK_Kick;
                    DISK_Kick = -1;
                    int swap_disk = PAGETABLE[swap_out_process_int][swap_out_vpn].pfn_or_dbi;
                    // Check to kick out TLB
                    if (swap_out_process_int == Process_C - 'A')
                        TLB_Clean_single(swap_out_vpn);
                    // PUT in TLB
                    TLB_Policy(VPN, pfn, Process_C - 'A');
                    // Physical Memory
                    PHYSICAL_MEMORY[pfn].clock = 1;
                    PHYSICAL_MEMORY[pfn].process_int = Process_C - 'A';
                    PHYSICAL_MEMORY[pfn].vpn = VPN;

                    // To FIFO queue
                    Pqueue = Pnode_Push(pfn);
                    // To Output
                    fprintf(fW, "Process %c, TLB Miss, Page Fault, %d,", Process_C, pfn);
                    fprintf(fW, " Evict %d of Process %c to %d, %d<<-1\n", swap_out_vpn, 'A' + swap_out_process_int, swap_disk, VPN);
                    fprintf(fW, "Process %c, TLB Hit, %d=>%d\n", Process_C, VPN, pfn);

                    //  For Analysis : LookUp Again
                    LookUp[Process_C - 'A']++;
                    TLBHit[Process_C - 'A']++;
                    PageFault[Process_C - 'A']++;
                }
            }
            else // TLB miss Page Found
            {
                if (PAGETABLE[Process_C - 'A'][VPN].present == 1) // In Memory Page Hit
                {
                    int pfn = PAGETABLE[Process_C - 'A'][VPN].pfn_or_dbi;
                    // PUT in TLB
                    TLB_Policy(VPN, pfn, Process_C - 'A');
                    // Physical Memory
                    PHYSICAL_MEMORY[pfn].clock = 1;
                    PHYSICAL_MEMORY[pfn].process_int = Process_C - 'A';
                    PHYSICAL_MEMORY[pfn].vpn = VPN;

                    // To Output
                    fprintf(fW, "Process %c, TLB Miss, Page Hit, %d=>%d\n", Process_C, VPN, pfn);
                    fprintf(fW, "Process %c, TLB Hit, %d=>%d\n", Process_C, VPN, pfn);

                    //  For Analysis : LookUp Again
                    LookUp[Process_C - 'A']++;
                    TLBHit[Process_C - 'A']++;
                }
                else // In Disk
                {
                    int dbi = PAGETABLE[Process_C - 'A'][VPN].pfn_or_dbi; // DBI
                    int swap_out_pfn = frame_swap(Process_C - 'A');
                    int swap_out_vpn = PHYSICAL_MEMORY[swap_out_pfn].vpn;
                    int swap_out_process_int = PHYSICAL_MEMORY[swap_out_pfn].process_int;

                    // Put into Memory
                    PAGETABLE[Process_C - 'A'][VPN].present = 1;
                    PAGETABLE[Process_C - 'A'][VPN].pfn_or_dbi = swap_out_pfn;
                    // Kick out Memory
                    PAGETABLE[swap_out_process_int][swap_out_vpn].present = 0;
                    // Physical Memory
                    PHYSICAL_MEMORY[swap_out_pfn].clock = 1;
                    PHYSICAL_MEMORY[swap_out_pfn].process_int = Process_C - 'A';
                    PHYSICAL_MEMORY[swap_out_pfn].vpn = VPN;
                    // Put to Disk
                    if (DISK_Kick == -1)
                        PAGETABLE[swap_out_process_int][swap_out_vpn].pfn_or_dbi = DISK++;
                    else
                        PAGETABLE[swap_out_process_int][swap_out_vpn].pfn_or_dbi = DISK_Kick;
                    // Kick out memory
                    DISK_Kick = dbi;
                    int new_disk = PAGETABLE[swap_out_process_int][swap_out_vpn].pfn_or_dbi;
                    if (swap_out_process_int == Process_C - 'A')
                        TLB_Clean_single(swap_out_vpn);
                    TLB_Policy(VPN, swap_out_pfn, Process_C - 'A');
                    fprintf(fW, "Process %c, TLB Miss, Page Fault, %d,", Process_C, swap_out_pfn);
                    fprintf(fW, " Evict %d of Process %c to %d, %d<<%d\n", swap_out_vpn, swap_out_process_int + 'A', new_disk, VPN, dbi);
                    fprintf(fW, "Process %c, TLB Hit, %d=>%d\n", Process_C, VPN, swap_out_pfn);
                    // To FIFO queue
                    Pqueue = Pnode_Push(swap_out_pfn);

                    //  For Analysis : LookUp Again
                    LookUp[Process_C - 'A']++;
                    TLBHit[Process_C - 'A']++;
                    PageFault[Process_C - 'A']++;
                }
            }
        }
        // DEBUG();
        Last_Process_c = Process_C;
    }
    fclose(fR);
    fclose(fW);
    //fclose(fTLB);
    //fclose(fPTB);
    //fclose(fPM);
    Analysis();
    Clean();
    return 0;
}
void TLB_Policy(int new_vpn, int new_pfn, int process_int)
{
    // Check if full or not
    for (int i = 0; i < TLB_NUM; ++i)
    {
        if (TLB[i].vpn == -1)
        {
            TLB[i].vpn = new_vpn;
            TLB[i].pfn = new_pfn;
            TLB[i].lru = TLB_lru++;
            return;
        }
    }
    if (strncmp(TLB_POLICY, "LRU", 3) == 0)
    {

        int minn = TLB[0].lru;
        int idx = 0;
        for (int tlb_num = 1; tlb_num < TLB_NUM; ++tlb_num)
            if (TLB[tlb_num].lru < minn)
                minn = TLB[tlb_num].lru, idx = tlb_num;
        TLB[idx].vpn = new_vpn;
        TLB[idx].pfn = new_pfn;
        TLB[idx].lru = TLB_lru++;
        return;
    }
    else if (strncmp(TLB_POLICY, "RANDOM", 6) == 0)
    {
        int idx = rand() % TLB_NUM;
        TLB[idx].vpn = new_vpn;
        TLB[idx].pfn = new_pfn;
        TLB[idx].lru = 0;
        return;
    }
}

void Initialize_Config()
{
    FILE *fp = fopen("sys_config.txt", "r");
    int a = 0;
    a = a + fscanf(fp, "TLB Replacement Policy: %s\n", TLB_POLICY);
    a = a + fscanf(fp, "Page Replacement Policy: %s\n", PAGE_POLICY);
    a = a + fscanf(fp, "Frame Allocation Policy: %s\n", FRAME_POLICY);
    a = a + fscanf(fp, "Number of Processes: %d\n", &PROCESSES);
    a = a + fscanf(fp, "Number of Virtual Page: %d\n", &VIRTUAL_PAGE);
    a = a + fscanf(fp, "Number of Physical Frame: %d\n", &PHYSICAL_FRAME);
    fclose(fp);
    return;
}

void TLB_Clean()
{
    for (int i = 0; i < TLB_NUM; ++i)
        TLB[i].vpn = -1, TLB[i].pfn = -1, TLB[i].lru = 0;
    TLB_lru = 0;
    return;
}

void PAGETABLE_Create_Table(int process, int page)
{
    PAGETABLE = (struct PageTable **)malloc(process * sizeof(struct PageTable *));
    for (int i = 0; i < process; ++i)
    {
        PAGETABLE[i] = (struct PageTable *)malloc(page * sizeof(struct PageTable));
        for (int j = 0; j < page; ++j)
            PAGETABLE[i][j].pfn_or_dbi = -1, PAGETABLE[i][j].present = 0;
    }
    return;
}
void PHYSICAL_MEMORY_Create_Table(int frame)
{
    PHYSICAL_MEMORY = (struct physical *)malloc(frame * sizeof(struct physical));
    for (int i = 0; i < frame; ++i)
        PHYSICAL_MEMORY[i].process_int = -1, PHYSICAL_MEMORY[i].clock = 0, PHYSICAL_MEMORY[i].vpn = -1;
    return;
}
/* ------------------------------------------------------ INIT END ------------------------------------------------- */

/* TLB */
int TLB_check(int vpn)
{
    for (int i = 0; i < TLB_NUM; ++i)
    {
        if (TLB[i].vpn == vpn)
            return i;
    }
    return -1;
}

void TLB_Clean_single(int vpn)
{
    for (int i = 0; i < TLB_NUM; ++i)
    {
        if (TLB[i].vpn == vpn)
            TLB[i].vpn = -1, TLB[i].pfn = -1, TLB[i].lru = 0;
    }
}

int PAGETABLE_check(int process_int, int vpn)
{
    return PAGETABLE[process_int][vpn].pfn_or_dbi;
}

bool PHYSICAL_empty_check()
{
    if (PHYSICAL_MEMORY[Global_clock_pointer].vpn != -1)
        return true;
    return false;
}

Pnode *Pnode_Create(int Physical_Index)
{
    Pnode *node = (Pnode *)malloc(sizeof(Pnode));
    node->next = NULL;
    node->pFrame = Physical_Index;
    return node;
}
Pnode *Pnode_Push(int Physical_Index)
{
    if (Pqueue == NULL)
        return Pnode_Create(Physical_Index);
    else
    {
        Pnode *temp = Pqueue;
        while (temp->next != NULL)
            temp = temp->next;
        temp->next = Pnode_Create(Physical_Index);
        return Pqueue;
    }
}
int Pnode_Pop()
{
    int pFrame = Pqueue->pFrame;
    Pnode *tt = Pqueue;
    Pqueue = Pqueue->next;
    free(tt);
    return pFrame;
}
int Pnode_Pop_Local(int process_int)
{
    if (PHYSICAL_MEMORY[Pqueue->pFrame].process_int == process_int) // HEAD
        return Pnode_Pop();
    else
    {
        Pnode *temp_last = Pqueue->next;
        Pnode *temp = Pqueue->next;
        while (temp != NULL)
        {
            if (PHYSICAL_MEMORY[temp->pFrame].process_int == process_int)
            {
                int pFrame = temp->pFrame;
                temp_last->next = temp->next;
                free(temp);
                return pFrame;
            }
            temp_last = temp;
            temp = temp->next;
        }
    }
    return -1;
}
int frame_swap(int process_int)
{
    if (strncmp(PAGE_POLICY, "FIFO", 4) == 0)
    {
        if (strncmp(FRAME_POLICY, "GLOBAL", 6) == 0)
            return Pnode_Pop();
        else if (strncmp(FRAME_POLICY, "LOCAL", 6) == 0)
            return Pnode_Pop_Local(process_int);
    }
    else
    {
        if (strncmp(FRAME_POLICY, "GLOBAL", 6) == 0)
        {
            while (1)
            {
                if (PHYSICAL_MEMORY[Global_clock_pointer].clock == 1)
                    PHYSICAL_MEMORY[Global_clock_pointer].clock = 0;
                else
                {
                    Global_clock_pointer = (Global_clock_pointer + 1) % PHYSICAL_FRAME;
                    return (Global_clock_pointer - 1 + PHYSICAL_FRAME) % PHYSICAL_FRAME;
                }
                Global_clock_pointer = (Global_clock_pointer + 1) % PHYSICAL_FRAME;
            }
        }
        else if (strncmp(FRAME_POLICY, "LOCAL", 6) == 0)
        {
            while (1)
            {
                if (PHYSICAL_MEMORY[Local_clock_pointer[process_int]].process_int == process_int)
                {
                    if (PHYSICAL_MEMORY[Local_clock_pointer[process_int]].clock == 1)
                        PHYSICAL_MEMORY[Local_clock_pointer[process_int]].clock = 0;
                    else
                    {
                        Local_clock_pointer[process_int] = (Local_clock_pointer[process_int] + 1) % PHYSICAL_FRAME;
                        return (Local_clock_pointer[process_int] - 1 + PHYSICAL_FRAME) % PHYSICAL_FRAME;
                    }
                }
                Local_clock_pointer[process_int] = (Local_clock_pointer[process_int] + 1) % PHYSICAL_FRAME;
            }
        }
    }

    return 0;
}
