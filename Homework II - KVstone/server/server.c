#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <netdb.h>
#include <ctype.h>
#include <stdbool.h>
#include "types.h"
#include "sock.h"
typedef struct HashNode
{
    struct HashNode* next;
    char *key;
    char *value;
} HashNode;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int Hashing (char* Key);
HashNode* HashNode_Create(char *Key, char *Value);
HashNode* HashNode_Insert(HashNode* Root, char* Key,char* Value);
char* HashNode_Search(HashNode* Root,char *Key);
HashNode* HashNode_Delete(HashNode* Root,char *Key);
void HashNode_ShowTB();
HashNode* HashTable[26];
bool *thread_sum;

void *child(void *arg)
{
    printf("Into Thread\n");
    int* data = (int*)arg;
    char buf[300],key[300],value[300];
    int connect_fd = data[0];
    int thread_num = data[1];
    while(1)
    {
        if (0 > recv(connect_fd, buf, sizeof(buf), 0))
        {
            printf("fail");
            exit(EXIT_FAILURE);
        }
        else
        {
            printf("%s",buf);
            pthread_mutex_lock(&mutex);
            switch(buf[0])
            {
            case 'S':
                sscanf(buf,"SET %s %s",key,value);
                HashTable[Hashing(key)] = HashNode_Insert(HashTable[Hashing(key)],key,value);
                strcpy(buf, HashNode_Search(HashTable[Hashing(key)],key));
                HashNode_ShowTB();
                pthread_mutex_unlock(&mutex);
                while(send(connect_fd,buf,sizeof(buf),0) < 0)
                    printf("Retry\n");
                break;
            case 'G':
                sscanf(buf,"GET %s",key);
                strcpy(buf, HashNode_Search(HashTable[Hashing(key)],key));
                HashNode_ShowTB();
                pthread_mutex_unlock(&mutex);
                while(send(connect_fd,buf,sizeof(buf),0) < 0)
                    printf("Retry\n");
                break;
            case 'D':
                sscanf(buf,"DELETE %s",key);
                if(strlen(HashNode_Search(HashTable[Hashing(key)],key))==0)
                    strcpy(buf,"Fail");
                else
                {
                    HashTable[Hashing(key)] = HashNode_Delete(HashTable[Hashing(key)],key);
                    HashNode_ShowTB();
                    strcpy(buf,"Success");
                }
                while(send(connect_fd,buf,sizeof(buf),0) < 0)
                    printf("Retry\n");
                pthread_mutex_unlock(&mutex);
                break;
            case 'E':
                thread_sum[thread_num] = false;
                printf("Thread %d is Exit\n",thread_num);
                close(connect_fd);
                pthread_mutex_unlock(&mutex);
                pthread_exit(NULL);
            default:
                pthread_mutex_unlock(&mutex);
                break;
            }
        }
    }
}
int main(int argc, char **argv)
{
    //Create Hash Table
    for (int i = 0 ; i < 26; ++i)
    {
        HashTable[i] = (HashNode*)malloc(sizeof(HashNode));
        HashTable[i]->next = NULL;
    }
    char *server_port = 0;
    int opt = 0;
    /* Parsing args */
    while ((opt = getopt(argc, argv, "p:")) != -1)
    {
        switch (opt)
        {
        case 'p':
            server_port = malloc(strlen(optarg) + 1);
            strncpy(server_port, optarg, strlen(optarg));
            break;
        case '?':
            fprintf(stderr, "Unknown option \"-%c\"\n", isprint(optopt) ? optopt : '#');
            return 0;
        }
    }
    if (!server_port)
    {
        fprintf(stderr, "Error! No port number provided!\n");
        exit(1);
    }
    /* Open a listen socket fd */
    int listenfd __attribute__((unused)) = open_listenfd(server_port);
    /* Start coding your server code here! */
    struct sockaddr_storage clientaddr;
    socklen_t client_length = sizeof(struct sockaddr_storage);
    pthread_t threads[100];
    thread_sum = (bool*)malloc(100*sizeof(bool));
    for (int i = 0; i<100; ++i)
        thread_sum[i] = false;
    while(1)
    {
        int now_thread = 0;
        while(1)
        {
            if(thread_sum[now_thread]==false)
                break;
            now_thread++;
        }
        int connect_fd  = accept(listenfd,(SA*) &clientaddr,&client_length);
        if ( now_thread > 99 )
        {
            send(connect_fd,"Fail",sizeof("Fail"),0);
            close(connect_fd);
        }
        else
        {
            printf("now_thread %d\n",now_thread);
            send(connect_fd,"Success",sizeof("Success"),0);
            int data[2];
            data[0] = connect_fd;
            data[1] = now_thread;
            pthread_create(&threads[now_thread],NULL,child,(void*)data);
            thread_sum[now_thread] = true;
        }
    }
    return 0;
}
int Hashing(char* Key)
{
    int hash = 0;
    for(int i = 0 ; i<strlen(Key); ++i)
        hash += (int)Key[i] - (int)'a';
    hash %= 26;
    hash += 26;
    return hash % 26;
}
HashNode* HashNode_Create(char *Key, char *Value)
{
    HashNode* node = (HashNode*)malloc(sizeof(HashNode));
    node->next = NULL;
    node->key = (char*) malloc(sizeof(Key));
    strcpy(node->key,Key);
    node->value = (char*) malloc(sizeof(Value));
    strcpy(node->value,Value);
    return node;
}
HashNode* HashNode_Insert(HashNode* Root, char* Key,char* Value)
{
    if(Root == NULL)
        Root->next = HashNode_Create(Key,Value);
    else
    {
        HashNode* temp = Root;
        while(temp->next!=NULL)
        {
            temp = temp->next;
            if(strcmp(temp->key,Key) == 0)
                return Root;
        }
        temp->next = HashNode_Create(Key,Value);
    }
    return Root;
}
char* HashNode_Search(HashNode* Root,char *Key)
{
    if(Root->next == NULL)
        return "";
    else
    {
        HashNode* temp = Root->next;
        while(temp!=NULL)
        {
            if(strcmp(temp->key,Key) == 0)
                return temp->value;
            temp = temp->next;
        }
    }
    return "";
}
HashNode* HashNode_Delete(HashNode* Root, char* Key)
{
    HashNode* temp = Root->next;
    if(temp == NULL)
        return Root;
    else if (strcmp(temp->key,Key) == 0)
    {
        Root->next = temp->next;
        free(temp);
        return Root;
    }
    else
    {
        HashNode* temp_last = temp;
        temp = temp->next;
        while(temp!=NULL)
        {
            if(strcmp(temp->key,Key) == 0)
            {
                temp_last->next = temp->next;
                free(temp);
                break;
            }
            else
            {
                temp_last = temp;
                temp = temp->next;
            }
        }
    }
    return Root;
}
void HashNode_ShowTB()
{
    return;
    HashNode *temp;
    for(int i = 0; i < 26; i++)
    {
        temp = HashTable[i]->next;
        printf("HashTable[%2d]-> ",i);
        while(temp!=NULL)
        {
            printf("[%6s,%6s]-> ",temp->key,temp->value);
            temp = temp->next;
        }
        printf("NULL\n");
    }
    return;
}
