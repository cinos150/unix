//
// Created by username on 15.06.16.
//

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <stdbool.h>
#include "Utilities.h"
#include "message_flow.h"
#include "LinkedList.h"
#include "structures.h"


#define NMMAX 30
#define MAX_PREFERENCE_INPUT 3
#define NUM_OF_PREFERENCES 5


char * choose_label(char * option)
{
    if(strcmp(option,"main_menu")==0)
        return  "\n1. Set/update your profile"
                "\n2. View other user profile"
                "\n3. Find a matching profile"
                "\n4. Invite for a date"
                "\n5. Schedule date/dates\n";

    else if(strcmp(option,"write_login")==0)
        return "Write your login:\n";

    else if(strcmp(option,"write_password")==0)
        return "Write your password:\n";

    else if(strcmp(option,"wrong_password")==0)
        return "The password is incorrect:\n";

    else if(strcmp(option,"already_logged_user")==0)
        return "This user is currently logged it, sorry\n";

    else if(strcmp(option,"serach_user_intro")==0)
        return "0 - GO BACK\n1 - number and login for fast search\n2 - list all and choose later\n";

    else if(strcmp(option,"update_profile")==0)
        return "\n0) GO BACK"
                "\n1) sex - [M]ale/[F]emale"
                "\n2) weight - [30;250]"
                "\n3) age - [18-99]"
                "\n4) hair length - [L]ong / [S]hort"
                "\n5) eye color - [B]lue / [G]reen"
                "\nWrite number and appropriate letter/number of characteristics\n";

    else if(strcmp(option,"password_path")==0)
        return "/password.txt";

    else if(strcmp(option,"properties_path")==0)
        return "/properties.txt";

    else if(strcmp(option,"invalid_input")==0)
        return "\nYou did something wrong try again\n";

    else if(strcmp(option,"users_path")==0)
        return "Users/";

    else if(strcmp(option,"no_such_user")==0)
        return "\nThere is no such user\n";

    else if(strcmp(option,"next_user")==0)
        return "\n[E]xit\n[N]ext\n";


    return NULL;
}


int communication_flow(int *communication_progress, ThreadArg *args, size_t size, char *response_message,
                        int *existing_user, char *filepath)
{
    int option =0;
    int status =0;

    char * path_to_users = strcat(filepath, choose_label("users_path"));

    switch (*communication_progress) {
        case 0:
            if(search_in_list(response_message,NULL) !=NULL) {
                send_message(args->socket, choose_label("already_logged_user"));
                return 0;
            }
            *existing_user =  login_part(args,size,response_message,path_to_users);
             (++*communication_progress);
        break;
        case 1:
            status= password_part(args, existing_user, response_message,path_to_users, choose_label("main_menu"));

             (status== -1) ?*communication_progress =0:++(*communication_progress);
            break;
        case 2:
            option = response_message[0] - '0';

            switch (option)
            {
                case 1:

                    update_profile(args,path_to_users);
                    send_message(args->socket,choose_label("main_menu"));
                    break;
                case 2:
                    send_message(args->socket,choose_label("serach_user_intro"));

                    see_other_profiles_func(args->socket,args->login, filepath);


                    break;
                case 3:
                    break;
                case 4:
                    break;
                case 5:
                    break;

                default:
                    send_message(args->socket,choose_label("invalid_input"));
            }

            break;
        default:
            send_message(args->socket,choose_label("invalid_input"));
    }

    return 0;

}


void see_other_profiles_func(int socket, char *login, char *filepath)
{
    int number_choosen =0;


    char *directories[NMMAX];
    char result[NMMAX];
    int size =0;


    while(true) {
        if ((size = (int) recive_message(socket, result)) == 0)
            break;
        result[size - 2] = '\0';
        strip(result);


        number_choosen = result[0] - '0';

        switch (number_choosen)
        {
            case 0:
                send_message(socket,choose_label("main_menu"));
                return;
            case 1:
                 search_by_login(socket,result, filepath, directories);
                break;
            case 2:
                search_one_by_one(socket,result,login,filepath,directories);
                break;
            default:
                send_message(socket,choose_label("invalid_input"));
                continue;
        }

    }


}


void search_one_by_one(int socket,char *message, char *login,char *filepath, char *allUsersLogins[])
{
    int num_of_users =0;
    if( strlen(message) > 1) {
        send_message(socket, choose_label("invalid_input"));
        return;
    }
    num_of_users = listdir(filepath,0,allUsersLogins);

    allUsersLogins[num_of_users] = NULL;
    int length_of_all_users =0;

    for( int i = 0; i < num_of_users; i++)
        length_of_all_users += strlen(allUsersLogins[i]);

    size_t size_messagge =0;


    char  response_message[NMMAX];
    int count_user_directory =0;
    char temparr[NMMAX];
    snprintf(temparr, strlen(allUsersLogins[count_user_directory])+1, "%s", allUsersLogins[count_user_directory]);
    char * result = choose_specific_user_from_list(temparr, filepath);
    int user_count =0;

    while(true) {
        send_message(socket, allUsersLogins[user_count++%num_of_users]);
        send_message(socket, result);
        send_message(socket, choose_label("next_user"));
        free(result);


        if((size_messagge = recive_message(socket, response_message))==0)
            break;

        response_message[size_messagge-2] = '\0';
        if(strlen(response_message) > 1 || strlen(response_message) ==0) {
            send_message(socket, choose_label("invalid_input"));
            continue;
        }

        result = next_exit_user_search(response_message[0],allUsersLogins, ++count_user_directory,filepath,num_of_users);
    if(result ==NULL)
    {
        send_message(socket,choose_label("serach_user_intro"));
        return;
    }
        num_of_users = listdir(filepath,0,allUsersLogins);

    }

}

char * next_exit_user_search(char option,char *allUsersLogins[], int countUser, char *filepath, int num_of_users)
{

    char temparr[NMMAX];
    memset(temparr,0,sizeof(temparr));
    switch(option)
    {
        case 'E':
        case 'e':
            return NULL;
        case 'N':
        case 'n':

            snprintf(temparr,strlen(allUsersLogins[countUser%num_of_users]) +1,"%s",allUsersLogins[countUser%num_of_users]);
            return  choose_specific_user_from_list(temparr, filepath);
    }
    return NULL;
}


void search_by_login(int socket, char * message, char *filepath, char *allUserNames[])
{
    int flag =0;
    char choosen_name[NMMAX];
    int num_of_users =0;
    if(strlen(message)<=1)
    {
        send_message(socket,choose_label("invalid_input"));
        return;
    }
    memcpy(choosen_name,&message[1],strlen(message));
    num_of_users = listdir(filepath,0,allUserNames);
    allUserNames[num_of_users] = NULL;

    for(int i =0 ;i < num_of_users; i++)
    {
        if(strcmp(choosen_name,allUserNames[i]) ==0)
        {
            char * result = choose_specific_user_from_list(choosen_name, filepath);

            send_message(socket, result);
            send_message(socket,choose_label("serach_user_intro"));
            free(result);
            flag = 1;
            break;
        }
    }
    if(flag ==0)
        send_message(socket,choose_label("no_such_user"));
}



char * choose_specific_user_from_list( char *chosen_user, char *filepath)
{
    int length_of_elements =0;
    char chos_user[strlen(chosen_user) +1];
    memset(chos_user,0,sizeof(chos_user));
    snprintf(chos_user,sizeof(chos_user),"%s",chosen_user);

    char *labels[5];
    labels[0] = "Sex: ";
    labels[1] = "Weight: ";
    labels[2] = "Age: ";
    labels[3] = "Hair Color: ";
    labels[4] = "Eye Color: ";


    int size_of_labels =0;

    char properties[strlen(filepath)+strlen(choose_label("properties_path")) + strlen(chos_user) + 1];
    memset(properties,0,sizeof(properties));
    char *user_prop[NUM_OF_PREFERENCES];
    for(int i =0; i < NUM_OF_PREFERENCES; i++) {
        user_prop[i] = (char *) malloc(NMMAX);
        size_of_labels +=strlen(labels[i] );
    }
    snprintf(properties,sizeof(properties),"%s%s%s",filepath,chos_user,choose_label("properties_path"));
    int j =0;

    char *file_content;
    FILE *pFile;
    pFile = fopen(properties,"r");
    size_t filesize =0;
    fseek(pFile, 0L, SEEK_END);
    filesize = (size_t) ftell(pFile);
    rewind(pFile);

    if ((file_content = (char *)malloc( (size_t) (filesize + 1))) == NULL)
        perror("Malloc:");

    fread(file_content,filesize,1,pFile);
    pclose(pFile);
    pFile = NULL;
    int counter =0;
    for(int i =0; i < filesize && counter < NUM_OF_PREFERENCES; i++)
    {
            while(file_content[++i]!= ';')
            {
                if(file_content[i] != ' ')
                 user_prop[counter][j++] = file_content[i];
            }
            user_prop[counter][j] = '\0';
            length_of_elements +=strlen(user_prop[counter]);
            j=0;
            counter++;
            --i;

    }

    free(file_content);

return concat_2d_array_to_string(user_prop,length_of_elements,NUM_OF_PREFERENCES,labels,size_of_labels);


}

void update_profile(ThreadArg *args, char *filepath)
{
    char letter_chosen = 0;
    char result[NMMAX];
    int number_choosen=0;
    int numerical_walues=0;
    enum options_enum {zero, sex, weight,age,hair_length,eye};

    int size =0;
    char *inv_inp = choose_label("invalid_input");
    char *inv_profiles = choose_label("update_profile");

    char *options_after_wrong_input = (char*)malloc(sizeof(inv_inp)+ sizeof(inv_profiles) -2) ;
    strcat(options_after_wrong_input,inv_inp);
    strcat(options_after_wrong_input,inv_profiles);


    send_message(args->socket,choose_label("update_profile"));

    while(true)
    {
        if((size = (int) recive_message(args->socket, result)) == 0)
            break;

        result[size - 2] = '\0';
        strip(result);


        number_choosen = result[0] - '0';

        switch(number_choosen){
            case zero:
                return;
            case weight:
            case age:
                numerical_walues = sublist_number(result);
                break;
            case hair_length:
            case sex:
            case eye:
                letter_chosen = sublist_letter(result);
                break;
            default:
                send_message(args->socket, options_after_wrong_input);
               break;


        }


        switch(number_choosen)
        {
            case sex:

                if(fill_position_char(letter_chosen, 'm','M','f','F')) {

                    find_and_replace_in_file(args,filepath,sex,letter_chosen, -1);
                    send_message(args->socket, choose_label("update_profile"));
                }
                else
                    send_message(args->socket, options_after_wrong_input);
                break;
            case weight:
                if(fill_position_number(numerical_walues, 30,250)) {
                    find_and_replace_in_file(args, filepath, weight, 0, numerical_walues);
                    send_message(args->socket, choose_label("update_profile"));
                }
                else
                    send_message(args->socket, options_after_wrong_input);
                break;
            case age:
                if(fill_position_number(numerical_walues, 18,99)){
                    find_and_replace_in_file(args, filepath, age, 0, numerical_walues);
                    send_message(args->socket, choose_label("update_profile"));
                }
                else
                    send_message(args->socket, options_after_wrong_input);
                break;
            case hair_length:
                if(fill_position_char(letter_chosen, 'l','L','s','S')) {
                    find_and_replace_in_file(args,filepath,hair_length,letter_chosen,-1);
                    send_message(args->socket, choose_label("update_profile"));
                }
                else
                    send_message(args->socket, options_after_wrong_input);
                break;
            case eye:
                if(fill_position_char(letter_chosen, 'b','B','g','G')) {
                    find_and_replace_in_file(args,filepath,eye,letter_chosen,-1);
                    send_message(args->socket, choose_label("update_profile"));
                }
                else
                    send_message(args->socket, options_after_wrong_input);
                break;


            default:
                break;
        }
        free(options_after_wrong_input);
    }
}



void find_and_replace_in_file(ThreadArg *args,char *filepath, int position_to_replace, char toreplace_letter, int toreplace_number)
{
    char numtostr[MAX_PREFERENCE_INPUT];
    char *property_path = choose_label("properties_path");
    char *user_login = args->login;
    int filesize=0;
    char full_properties_path[strlen(filepath) + strlen(property_path)+strlen(user_login) +1];
    snprintf(full_properties_path,sizeof(full_properties_path),"%s%s%s",filepath,user_login,property_path);

    if(toreplace_number != -1) {
        sprintf(numtostr,"%d",toreplace_number);
    }

    char *file_cont = NULL;
    file_cont = lock_and_read(file_cont,full_properties_path, &filesize);


                (toreplace_number != -1)?
                    replace_old_characteristics_with_number(filesize,file_cont,position_to_replace,numtostr)
    :replace_old_characteristics_letter(filesize,file_cont, position_to_replace, toreplace_letter);

     lock_and_write( file_cont, full_properties_path);




    }


char * lock_and_read( char *content, char *path_to_file, int * filesize) {

    FILE *fp;
    if ((fp = fopen(path_to_file, "r")) == NULL)
        ERR("writing user property");


    while (1) {
        if (ftrylockfile(fp) == 0) {
            if (fp == NULL)
                ERR("Locking file");
            else {


                fseek(fp, 0L, SEEK_END);
                *filesize = (int) ftell(fp);
                rewind(fp);

                if((content = calloc(1,*filesize))==NULL)
                    ERR("calloc");

                fread(content,*filesize,1,fp);

                funlockfile(fp);

                fclose(fp);
            }
            break;

        }
    }
    return content;
}


void lock_and_write( char *content, char *path_to_file) {


    FILE *fp;
    if ((fp = fopen(path_to_file, "w+")) == NULL)
        ERR("reading user property");



    while (1) {
        if (ftrylockfile(fp) == 0) {
            if (fp == NULL)
                ERR("Locking file");
            else {
                fprintf(fp,"%s",content);
                funlockfile(fp);
                fclose(fp);
            }
            break;
        }
    }
}



void replace_old_characteristics_with_number(int filesize,char*file_content, int number, char *num_to_string)
{
    int j =0;
    int counter =0;
    for(int i =0; i < filesize ; i++)
    {
        if(file_content[i] != ';')
            continue;
            counter++;
            if(counter != number)
                continue;

        while(j<MAX_PREFERENCE_INPUT) {
            file_content[i+j+1] = (char) ((num_to_string[j] != '\0') ?
                                          num_to_string[j] : ' ');
                    j++;
        }
        return;

    }
}


void replace_old_characteristics_letter(int filesize,char*file_content, int number, char toreplace_letter)
{
    int counter =0;
    for(int i =0; i < filesize ; i++)
    {
        if(file_content[i] != ';')
            continue;
            counter++;

            if(counter == number)
            {
                file_content[1+i] = toreplace_letter;
                return;
            }

    }
}


int login_part(ThreadArg *args, size_t size, char *response_message,char* path_to_users )
{
    char *ask_for_password = "Write your password: \n";
    int if_user_exists =0;
    char *path_to_specific_user;

    if ((args->login = (char *) malloc(sizeof(size - 2))) == NULL)
        perror("Malloc:");

    strcpy(args->login,response_message);


    create_dir(path_to_users);


    path_to_specific_user = strcat(path_to_users, args->login);
    if_user_exists = create_dir(path_to_specific_user);

    send_message(args->socket,ask_for_password);


    return if_user_exists;
}



int password_part(ThreadArg *args, int *existing_user, char *response_message, char *pathcurr, char *options)
{
    char *tempPath;
    FILE *pFile = NULL;
    int output_value;

    strcat(pathcurr,args->login);

    if ((tempPath= (char *) malloc(strlen(pathcurr))) == NULL)
        perror("Malloc:");

    strcpy(tempPath, pathcurr);
    output_value = (*existing_user == 0)?
                                  new_user(args,pFile,tempPath,response_message,options)
                                  :user_exists(args,pFile,response_message,options,tempPath);

    free(tempPath);
    return output_value;
}

int user_exists(ThreadArg *args,FILE *pFile, char * response_message, char *options,char *pathcurr)
{
    char *wrong_password = "Wrong password \nWrite your login:\n";
    char * password = choose_label("password_path");

    char path_password[strlen(pathcurr) + strlen(password)+1];
    snprintf(path_password,sizeof(path_password),"%s%s",pathcurr,password);


    char * existing_password;


    existing_password = read_all_from_file(pFile,path_password);
    if (strcmp(response_message, existing_password) == 0) {


        send_message(args->socket,options);

        free(existing_password);
        return 1;
    }
    else {
        send_message(args->socket,wrong_password);
        strcpy(args->login,"");
        free(existing_password);
        return -1;
    }

}


int new_user(ThreadArg *args,FILE *pFile, char *pathcurr, char *response_message, char *options)
{
    char *password_end_path = "/password.txt";
    char *property_path = "/properties.txt";
    char properties[strlen(pathcurr)+strlen(property_path) +1];
    char password[strlen(pathcurr) + strlen(password_end_path) +1];

    snprintf(properties,sizeof(properties),"%s%s",pathcurr,property_path);
    snprintf(password,sizeof(password),"%s%s",pathcurr,password_end_path);


    pFile = fopen(password, "w");

    if (pFile != NULL) {
        fprintf(pFile, "%s", response_message);
        fclose(pFile);
        pFile = fopen(properties, "w");

        fprintf(pFile, "%s", ";   ;   ;   ;   ;   ;");
        fclose(pFile);
        send_message(args->socket,options);

        return 0;
    }



    return 1;
}

