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


char * lock_and_read( char *content, char *path_to_file, int * filesize) ;
char * choose_specific_user_from_list( char chosen_user[], char *filepath);
void lock_and_write( char *content, char *path_to_file) ;

void search_by_login(int socket, char * message, char *filepath, char *allUserNames[]);
void find_and_replace_in_file(ThreadArg *args,char *filepath, int position_to_replace, char toreplace_letter, int toreplace_number);
void see_other_profiles_func(int socket, char *login, char *filepath);
void search_one_by_one(int socket,char *message, char *login,char *filepath, char *allUsersLogins[]);
char * next_exit_user_search(char option,char *allUsersLogins[], int countUser, char *filepath, int num_of_users);
void replace_old_characteristics_with_number(int filesize,char*file_content, int number, char *num_to_string);
void replace_old_characteristics_letter(int filesize,char*file_content, int number, char toreplace_letter);

void update_profile(ThreadArg *args, char *filepath);

int communication_flow(int *communication_progress, ThreadArg *args, size_t size, char *response_message,
                        int *existing_user, char *filepath);
int login_part(ThreadArg *args, size_t size, char *response_message,char* path_to_users );
int password_part(ThreadArg *args, int *existing_user, char *response_message, char *pathcurr, char *string);
int user_exists(ThreadArg *args,FILE *pFile, char * response_message, char *options,char *pathcurr);
int new_user(ThreadArg *args,FILE *pFile, char *pathcurr, char *response_message, char *options);

#endif