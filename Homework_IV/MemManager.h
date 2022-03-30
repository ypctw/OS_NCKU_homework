#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#define TLB_NUM 32
#define min(x, y) (x < y ? x : y)
struct physical
{
    int process_int, clock, vpn;
};
typedef struct Pnode
{
    int pFrame;
    struct Pnode *next;
} Pnode;
struct PageTable
{
    int pfn_or_dbi;
    // int reference;
    int present;
};
struct tlb
{
    int vpn;
    int pfn;
    int lru;
} tlb;
int Global_clock_pointer;
int *Local_clock_pointer;

/*-------------*/
double *TLBHit;
double *LookUp;
double *PageFault;
double *Reference;
/* --------- Physical Queue ---------*/
struct Pnode *Pqueue;
FILE *fPM;
FILE *fPTB;
FILE *fTLB;

/* ------------------------------------------------------ INIT ----------------------------------------------------- */

/******  Config  ******/
char TLB_POLICY[10];
char PAGE_POLICY[10];
char FRAME_POLICY[10];
int PROCESSES;
int VIRTUAL_PAGE;
int PHYSICAL_FRAME;
/**********************/

struct tlb TLB[TLB_NUM]; // TLB
int TLB_count;
int TLB_Status;
int TLB_lru;
struct PageTable **PAGETABLE; // Page Table

struct physical *PHYSICAL_MEMORY; // Physical Memory Frame

/*------- DISK ------*/
int DISK;
int DISK_Kick;
void Initialize();
void TLB_Policy(int new_vpn, int new_pfn, int process_int);
void Initialize_Config();
void TLB_Clean();
void PAGETABLE_Create_Table(int process, int page);
void PHYSICAL_MEMORY_Create_Table(int frame);
int TLB_check(int vpn);
void TLB_Clean_single(int vpn);
int PAGETABLE_check(int process_int, int vpn);
bool PHYSICAL_empty_check();
Pnode *Pnode_Create(int Physical_Index);
Pnode *Pnode_Push(int Physical_Index);
int Pnode_Pop();
int Pnode_Pop_Local(int process_int);
int frame_swap(int process_int);
void Clean();