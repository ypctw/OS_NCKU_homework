# HW2 Simple Key-value Store

## Directories
- /server ->	server program related sources
- /client ->	client program related sources
- /common ->	common inclusions
- /util ->	common utilization
- /build ->	target build directory

## Building the Project
Code out your `/server/server.c` and `/client/client.c`, then
```shell
$ make
```
Test your `/build/server` and `build/client`.

## Implementations
### Please briefly describe your multi-threading design pattern
1.  Use `pthread_t threads[10]` array and `bool *thread_sum` array to record the pthread
2.  Once server `accept` the client, the server use `pthread_create` to carry out the multi-thread.
3.  If Clients are up to 10, the server will not accept new client.
4.  Use `pthread_mutex_lock` and `pthread_mutex_unlock` to avoid race condition.
### Please briefly describe your data structure implementation
#### DataNode
```c
typedef struct HashNode
{
    struct HashNode* next;
    char *key;
    char *value;
} HashNode;
```
1. Use Hash Table as the DataBase. a linked list array (size = 26)
2. `int Hashing (char* Key);` : Use Key to create Hash value (0-25)
3. `HashNode* HashNode_Create(char *Key, char *Value);` Create a Link-list Node
4.  `HashNode* HashNode_Insert(HashNode* Root, char* Key,char* Value);` : Save K-V pair to Hash Table (as a linked list).
5.  `HashNode* HashNode_Search(HashNode* Root,char *Key);` : Search the Key and Return Value.
6.  `HashNode* HashNode_Delete(HashNode* Root,char *Key);` : Delete the K-V pair.

## References
* [POSIX thread man pages](https://man7.org/linux/man-pages/man7/pthreads.7.html)
* [socket man pages](https://linux.die.net/man/7/socket)

