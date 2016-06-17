//
// Created by username on 15.06.16.
//

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "Utilities.h"
#include "message_flow.h"

#define NMMAX 30



void communication_flow(int *communication_progress, ThreadArg *args, size_t size, char *response_message,
                        int *existing_user, char *filepath)
{
    int option;
    int status;
    char *no_option = "There is no such option\n";



    switch (*communication_progress) {
        case 0:
            *existing_user =  login_part(args,size,response_message,filepath);
             (++*communication_progress);
        break;
        case 1:
            status= password_part(args,existing_user,response_message,filepath);

             if(status== -1)
                 *communication_progress =0;
             else {

                 ++(*communication_progress);
             }
            break;
        case 2:
            option = response_message[0] - '0';

            switch (option)
            {
                case 1:

                    break;
                case 2:
                    break;
                case 3:
                    break;
                case 4:
                    break;
                case 5:
                    break;
                case 6:
                    break;
                default:
                    if (TEMP_FAILURE_RETRY(send(args->socket, no_option, strlen(no_option), 0)) == -1)
                        ERR("write");
            }

            break;
    }

}


int login_part(ThreadArg *args, size_t size, char *response_message,char* pathcurr )
{
    char *ask_for_password = "Write your password: \n";
    int user_exists =0;
    char *temp_path;
    if ((args->user_info->login = (char *) malloc(sizeof(size - 2))) == NULL)
        perror("Malloc:");

    strcpy(args->user_info->login,response_message);
    temp_path = strcat(pathcurr, args->user_info->login);

    if (mkdir(temp_path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == -1) {
        if (EEXIST == errno) {
            user_exists = 1;
        }
    }

    if (TEMP_FAILURE_RETRY(send(args->socket, ask_for_password, strlen(ask_for_password), 0)) == -1)
        ERR("write");

    return user_exists;
}

int password_part(ThreadArg *args, int *existing_user,char *response_message,char* pathcurr)
{

    FILE *pFile = NULL;
    char *options =
            "You are logged:"
                    "\n1. Set/update your profile"
                    "\n2. View other user profile"
                    "\n3. Find a matching profile"
                    "\n4. Invite for a date"
                    "\n5. Schedule date/dates"
                    "\n6. Log out\n";

    if ((args->user_info->password = (char *) malloc(sizeof(response_message))) == NULL)
        perror("Malloc:");


    strcat(pathcurr, "/password.txt");

    if (*existing_user == 0) {
        return  new_user(args,pFile,pathcurr,response_message,options);

    }
    else {
        return user_exists(args,pFile,response_message,options,pathcurr);
    }
}

int user_exists(ThreadArg *args,FILE *pFile, char * response_message, char *options,char *pathcurr)
{
    char *wrong_password = "Wrong password \nWrite your login:\n";
    int filesize;
    char * existing_password;

    pFile = fopen(pathcurr, "r");
    if (pFile == NULL) {
        ERR("reading file");
    }
    fseek(pFile, 0L, SEEK_END);
    filesize = ftell(pFile);
    rewind(pFile);

    existing_password = calloc(1, filesize + 1);
    if (!existing_password) {
        fclose(pFile);
        ERR("reading from file");
    }

    fscanf(pFile, "%s", existing_password);
    fclose(pFile);

    if (strcmp(response_message, existing_password) == 0) {
        args->user_info->password = response_message;
        if (TEMP_FAILURE_RETRY(send(args->socket, options, strlen(options), 0)) == -1)
            ERR("write");

        free(existing_password);
        return 1;
    }
    else {
        if (TEMP_FAILURE_RETRY(send(args->socket, wrong_password, strlen(wrong_password), 0)) == -1)
            ERR("write");

        free(existing_password);
        return -1;
    }

}

int new_user(ThreadArg *args,FILE *pFile, char *pathcurr, char *response_message, char *options)
{
    pFile = fopen(pathcurr, "w");

    if (pFile != NULL) {
        fprintf(pFile, "%s", response_message);
        fclose(pFile);
        strcpy(args->user_info->password,response_message);

        if (TEMP_FAILURE_RETRY(send(args->socket, options, strlen(options), 0)) == -1)
            ERR("write");

        return 0;
    }
    return 1;
}
