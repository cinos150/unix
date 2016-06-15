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

ssize_t bulk_read(int, char *, size_t);

ssize_t bulk_write(int, char *, size_t);

void sethandler(void (*)(int), int);

#endif