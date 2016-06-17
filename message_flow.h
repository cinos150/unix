//
// Created by username on 15.06.16.
//


#ifndef MESSAGE_FLOW_H
#define MESSAGE_FLOW_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "structures.h"


void communication_flow(int *communication_progress, ThreadArg *args, size_t size, char *response_message,
                        int *existing_user, char *filepath);
int login_part(ThreadArg *args, size_t size, char *response_message,char* pathcurr );
int password_part(ThreadArg *args, int *existing_user,char *response_message,char* pathcurr);
int user_exists(ThreadArg *args,FILE *pFile, char * response_message, char *options,char *pathcurr);
int new_user(ThreadArg *args,FILE *pFile, char *pathcurr, char *response_message, char *options);


#endif