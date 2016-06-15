//
// Created by username on 15.06.16.
//
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <signal.h>
#include <netdb.h>
#include "Utilities.h"

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
