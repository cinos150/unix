
#ifndef UNIX_UTILITIES_H
#define UNIX_UTILITIES_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "structures.h"

void check_and_fill_letter(ThreadArg *args,char letter_chosen, char a, char A, char b, char B, char * filepath, int option, char *wrong_input);
void check_and_fill_number(ThreadArg *args,int numerical_walues, int left_range , int right_range,char * filepath,int option, char *wrong_input );
void find_and_replace_in_file(ThreadArg *args, char *filepath, int position_to_replace, char toreplace_letter,int toreplace_number);
void replace_old_characteristics_with_number(int filesize, char *file_content, int number, char *num_to_string);
void replace_old_characteristics_letter(int filesize, char *file_content, int number, char toreplace_letter);
char * choose_label(char * option);
char * lock_and_read( char *content, char *path_to_file, int * filesize) ;
void lock_and_write( char *content, char *path_to_file, char *mode);
int fill_position_number(int chosen, int range_left, int range_right);
int fill_position_char(char chosen, char a, char A, char b, char B);
char * concat_2d_array_to_string(char *twodimarray[], int size_of_content,int num_of_elem, char *labels[] , int size_of_labels);
int create_dir(char *path_to_file);
int  sublist_number(char* pstring);
char  sublist_letter(char* pstring);
int  listdir(const char *name,char *my_name, int look_for_his_own, char *directories[]);
size_t recive_message(int socket, char *message);
void send_message(int socket, char* message);
char * cutString(char *fullpath, char fromCrop);
void strip(char *s);
void sethandler(void (*)(int), int);

#endif