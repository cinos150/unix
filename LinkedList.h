//
// Created by username on 15.06.16.
//


#ifndef LINKED_LIST_H
#define LINKED_LIST_H


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "structures.h"


ThreadArg* create_list(int clientfd, char *filepath);
ThreadArg* add_to_list(int clientfd, char *filepath);
ThreadArg* search_in_list(char *login, ThreadArg **prev);
int delete_from_list(char *login);




#endif