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

void main_menu_flow(int option, ThreadArg * args, char *path_to_users, char * filepath);
char * ask_for_a_date(int socket, char *filepath, char *login);
void find_in_other_users(int socket, int num_of_users, char * my_attribute, int chosen_attr, char *filepath, char *directories[]);
void find_matching_profile(int socket, char *login,char *filepath) ;
char * read_specific_attribute_from_string(int number, int filesize, char *file_content);
int display_chosen_user(int socket,char * result,int num_of_users, char * filepath );
char * choose_specific_user_from_list(const char chosen_user[], char *filepath);
int update_specific_characteristics(ThreadArg *args, int number_choosen, char letter_chosen, int numerical_walues,char *filepath, char *options_after_wrong_input);
void search_by_login(int socket, char * message, char *filepath);
void find_and_replace_in_file(ThreadArg *args,char *filepath, int position_to_replace, char toreplace_letter, int toreplace_number);
void see_other_profiles_func(int socket, char *login, char *filepath);
void search_one_by_one(int socket,char *message, char *login,char *filepath, char *allUsersLogins[]);
char * next_exit_user_search(char option, int countUser, char *filepath);
void update_profile(ThreadArg *args, char *filepath);
int communication_flow(int *communication_progress, ThreadArg *args, size_t size, char *response_message,int *existing_user, char *filepath);
int login_part(ThreadArg *args, size_t size, char *response_message,char* path_to_users );
int password_part(ThreadArg *args, int *existing_user, char *response_message, char *pathcurr, char *string);
int user_exists(ThreadArg *args, char *response_message, char *options, char *pathcurr);
int new_user(ThreadArg *args, char *pathcurr, char *options,char *password);
char * show_users_avatar(char* filepath, char* chosen_name);


#endif