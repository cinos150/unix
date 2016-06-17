//
// Created by username on 15.06.16.
//


#ifndef UNIX_UTILITIES_H
#define UNIX_UTILITIES_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>

#define MAXBUFFER 1024
#define DEFAULTPORT 8080

char * CutFileName(char * fullpath, char fromCrop);

void strip(char *s);


char * readline(int fd);

void sethandler(void (*)(int), int);

#endif