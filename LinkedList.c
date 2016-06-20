#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include <string.h>
#include <pthread.h>
#include "structures.h"

ThreadArg *head = NULL;
ThreadArg *curr = NULL;

ThreadArg *create_list(int clientfd, char *filepath) {


    ThreadArg *threadArgHead = NULL;


    if ((threadArgHead = (ThreadArg *) malloc(sizeof(ThreadArg))) == NULL)
        perror("Malloc:");


    threadArgHead->socket = clientfd;
    threadArgHead->filepath = filepath;

    threadArgHead->next = NULL;

    head = curr = threadArgHead;


    return threadArgHead;
}

ThreadArg *add_to_list(int clientfd, char *filepath) {

    if (NULL == head) {
        return (create_list(clientfd, filepath));
    }


    ThreadArg *threadArgHead = NULL;


    if ((threadArgHead = (ThreadArg *) malloc(sizeof(ThreadArg))) == NULL)
        perror("Malloc:");


    if (pthread_mutex_init(&threadArgHead->lock, NULL) != 0)
        ERR("Mutex init");


    threadArgHead->socket = clientfd;
    threadArgHead->filepath = filepath;


    threadArgHead->next = NULL;


    curr->next = threadArgHead;
    curr = threadArgHead;


    return threadArgHead;
}

ThreadArg *search_in_list(char *login, ThreadArg **prev) {
    ThreadArg *ptr = head;
    ThreadArg *tmp = NULL;
    bool found = false;


    while (ptr != NULL) {

        if (ptr->login != NULL && strcmp(ptr->login, login) == 0) {
            found = true;
            break;
        }
        else {
            tmp = ptr;
            ptr = ptr->next;
        }
    }

    if (true == found) {
        if (prev)
            *prev = tmp;
        return ptr;
    }
    else {
        return NULL;
    }
}

int delete_from_list(char *login) {
    ThreadArg *prev = NULL;
    ThreadArg *del = NULL;


    del = search_in_list(login, &prev);
    if (del == NULL) {
        return -1;
    }
    else {

        if (prev != NULL)
            prev->next = del->next;

        if (del == curr) {
            curr = prev;
        }
        if (del == head) {
            if (del->next == NULL) {
                head = NULL;
                curr = NULL;
            }
            else
                head = del->next;
        }
    }


    free(del);
    del = NULL;


    return 0;
}



