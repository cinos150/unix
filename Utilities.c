//
// Created by username on 15.06.16.
//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <assert.h>
#include "Utilities.h"


#define NMMAX 30


void strip(char *s)
{
    char *p2 = s;
    while(*s != '\0')
    {
        if(*s != '\t' && *s != '\n' && *s != '\r' && *s != '\\' && *s !=' ')
            *p2++=*s++;
        else
            ++s;
    }
    *p2 ='\0';
}


char * CutFileName(char * fullpath, char fromCrop)
{
    char *result;
    int i =strlen(fullpath);
    for(;i>=0;i--)
    {
        if(fullpath[i] == fromCrop)
        {
            result = (char*)malloc(i);
            memcpy(result,fullpath,i+1);
            return result;
        }

    }

    return fullpath;
}



void sethandler(void (*f)(int), int sigNo)
{
    struct sigaction act;
    memset(&act, 0x00, sizeof(struct sigaction));
    act.sa_handler = f;

    if (-1 == sigaction(sigNo, &act, NULL))
        perror("sigaction");
}
