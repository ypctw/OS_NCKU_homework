#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <ctype.h>
#include <stdlib.h>

#include "sock.h"
void Help()
{
    printf("-------------------------------------------------------------\n");
    printf("Connect\t\t\tDescription\n");
    printf("SET [key] [value]\tStore the K-V pair into the database.\n");
    printf("GET [key]\t\tGet value of [key] from database\n");
    printf("DELETE [key]\t\tDelete [key] from database\n");
    printf("EXIT\t\t\tEnd the Connection and Close\n");
    printf("-------------------------------------------------------------\n");
    return;
}
void ErrorMessage(char *s)
{
    printf("[Fail] Did you Mean %s ? TYPE HELP\n",s);
    return;
}

int main(int argc, char **argv)
{
    int opt;
    char *server_host_name = NULL, *server_port = NULL;
    /* Parsing args */
    while ((opt = getopt(argc, argv, "h:p:")) != -1)
    {
        switch (opt)
        {
        case 'h':
            server_host_name = malloc(strlen(optarg) + 1);
            strncpy(server_host_name, optarg, strlen(optarg));
            break;
        case 'p':
            server_port = malloc(strlen(optarg) + 1);
            strncpy(server_port, optarg, strlen(optarg));
            break;
        case '?':
            fprintf(stderr, "[INFO] Unknown option \"-%c\"\n", isprint(optopt) ?
                    optopt : '#');
            return 0;
        }
    }
    if (!server_host_name)
    {
        fprintf(stderr, "[INFO] Error!, No host name provided!\n");
        exit(EXIT_FAILURE);
    }
    if (!server_port)
    {
        fprintf(stderr, "[INFO] Error!, No port number provided!\n");
        exit(EXIT_FAILURE);
    }
    /* Open a client socket fd */
    int clientfd __attribute__((unused)) = open_clientfd(server_host_name, server_port);
    /* Start your coding client code here! */
    if (0 >= clientfd)
    {
        printf("[INFO] Connect Error\n");
        close(clientfd);
        exit(EXIT_FAILURE);
    }
    char buf[300];
    recv(clientfd, buf, sizeof(buf), 0);
    if(buf[0]=='F')
    {
        printf("[INFO] Connect Error\n");
        close(clientfd);
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("[INFO] Connet to %s : %s\n",server_host_name,server_port);
    }
    char key[300],value[300],type[300];
    printf("[INFO] Welcome ! please type HELP for available Command\n> ");
    while(1)
    {
        fgets(buf, sizeof(buf), stdin);
        switch(buf[0])
        {
        case 'H':
            sscanf(buf,"%s",type);
            if(strcmp(type,"HELP")!=0)
                ErrorMessage("HELP");
            else
                Help();
            break;
        case 'E':
            sscanf(buf,"%s",type);
            if(strcmp(type,"EXIT")!=0)
                ErrorMessage("EXIT");
            else
            {
                while(send(clientfd, buf, sizeof(buf), 0) < 0)
                    printf("Retry!\n");
                while(recv(clientfd, buf, sizeof(buf), 0) < 0)
                    printf("Retry!\n");
                close(clientfd);
                exit(EXIT_SUCCESS);
            }
            break;
        case 'S':
            sscanf(buf,"%s %s %s",type,key,value);
            if(strcmp(type,"SET") != 0)
                ErrorMessage("SET");
            else
            {
                while(send(clientfd, buf, sizeof(buf), 0) < 0)
                    printf("Retry!\n");
                while(recv(clientfd, buf, sizeof(buf), 0) < 0)
                    printf("Retry!\n");
                sscanf(buf,"%s",type);
                if (strcmp(type,value)==0)
                    printf("[ OK ] PUT K-V pair (%s, %s) to DataBase!\n",key,value);
                else
                    printf("[ERROR] K-V pair (%s, %s) exists\n",key,type);
            }
            break;
        case 'G':
            sscanf(buf,"%s %s",type,key);
            if(strcmp(type,"GET")!=0)
                ErrorMessage("GET");
            else
            {
                while(send(clientfd, buf, sizeof(buf), 0) < 0)
                    printf("Retry!\n");
                while(recv(clientfd, buf, sizeof(buf), 0) < 0)
                    printf("Retry!\n");
                if (strcmp(buf,"")==0)
                    printf("[ERROR] Cannot Find the Key !\n");
                else
                {
                    sscanf(buf,"%s",value);
                    printf("[ OK ] The Value of %s is %s !\n",key,value);
                }
            }
            break;
        case 'D':
            sscanf(buf,"%s %s",type,key);
            if(strcmp(type,"DELETE")!=0 &&strlen(key) != 6)
                ErrorMessage("DELETE");
            else
            {
                while(send(clientfd,buf,sizeof(buf),0) < 0)
                    printf("Retry!\n");
                while(recv(clientfd, buf, sizeof(buf), 0) < 0)
                    printf("Retry");
                if (buf[0] =='S')
                    printf("[ OK ] Key '%s' is deleted\n",key);
                else
                    printf("[ERROR] Key '%s' doesn't exist\n",key);
            }
            break;
        default:
            ErrorMessage("HELP");
            break;
        }
        printf("> ");
    }
    return 0;
}
