//
// Created by username on 15.06.16.
//
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <assert.h>
#include "Utilities.h"


char** str_split(char* a_str, const char a_delim)
{
    char** result    = 0;
    size_t count     = 0;
    char* tmp        = a_str;
    char* last_delimiter = 0;
    char delim[2];
    delim[0] = a_delim;
    delim[1] = 0;

    /* Count how many elements will be extracted. */
    while (*tmp)
    {
        if (a_delim == *tmp)
        {
            count++;
            last_delimiter = tmp;
        }
        tmp++;
    }

    /* Add space for trailing token. */
    count += last_delimiter < (a_str + strlen(a_str) - 1);

    /* Add space for terminating null string so caller
       knows where the list of returned strings ends. */
    count++;



    result = malloc(sizeof(char*) * count);

    if (result)
    {
        size_t idx  = 0;
        char* token = strtok(a_str, delim);

        while (token)
        {
            assert(idx < count);
            *(result + idx++) = strdup(token);
            token = strtok(0, delim);
        }
        assert(idx == count - 1);
        *(result + idx) = 0;
    }

    return result;
}


ssize_t bulk_read(int fd, char *buf, size_t count){
    int c;
    size_t len=0;
    do{
        c=TEMP_FAILURE_RETRY(read(fd,buf,count));
        if(c<0) return c;
        if(0==c) return len;
        buf+=c;
        len+=c;
        count-=c;
    }while(count>0);
    return len ;
}

ssize_t bulk_write(int fd, char *buf, size_t count){
    int c;
    size_t len=0;
    do{
        c=TEMP_FAILURE_RETRY(write(fd,buf,count));
        if(c<0) return c;
        buf+=c;
        len+=c;
        count-=c;
    }while(count>0);
    return len ;
}


void sethandler(void (*f)(int), int sigNo)
{
    struct sigaction act;
    memset(&act, 0x00, sizeof(struct sigaction));
    act.sa_handler = f;

    if (-1 == sigaction(sigNo, &act, NULL))
        perror("sigaction");
}
