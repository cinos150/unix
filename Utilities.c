//
// Created by username on 15.06.16.
//
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <assert.h>
#include <dirent.h>
#include <sys/stat.h>
#include "Utilities.h"
#include "structures.h"

#define NMMAX 30
#define MAX_PREFERENCE_INPUT 3
#define NUM_OF_PREFERENCES 5


char *choose_label(char *option) {


    if (strcmp(option, "main_menu") == 0)
        return "\n1. Set/update your profile"
                "\n2. View other user profile"
                "\n3. Find a matching profile"
                "\n4. Invite for a date"
                "\n5. Schedule date/dates\n";

    else if (strcmp(option, "pending") == 0)
        return "/pending.txt";

    else if (strcmp(option, "ask_a_date") == 0)
        return "\nwrite login of your chosen one\n";

    else if (strcmp(option, "write_login") == 0)
        return "Write your login:\n";

    else if (strcmp(option, "write_password") == 0)
        return "Write your password:\n";

    else if (strcmp(option, "wrong_password") == 0)
        return "The password is incorrect:\n";

    else if (strcmp(option, "already_logged_user") == 0)
        return "This user is currently logged it, sorry\n";

    else if (strcmp(option, "search_user_intro") == 0)
        return "0 - GO BACK\n1 - number and login for fast search\n2 - list them one by one\n";

    else if (strcmp(option, "update_profile") == 0)
        return "\n0) GO BACK"
                "\n1) sex - [M]ale/[F]emale"
                "\n2) weight - [30;250]"
                "\n3) age - [18-99]"
                "\n4) hair length - [L]ong / [S]hort"
                "\n5) eye color - [B]lue / [G]reen"
                "\nWrite number and appropriate letter/number of characteristics\n";
    else if (strcmp(option, "find_match_by") == 0)
        return "\n0) GO BACK"
                "\n1) sex"
                "\n2) weight"
                "\n3) age "
                "\n4) hair length "
                "\n5) eye color "
                "\nWrite number \n";
    else if (strcmp(option, "password_path") == 0)
        return "/password.txt";

    else if (strcmp(option, "properties_path") == 0)
        return "/properties.txt";

    else if (strcmp(option, "invalid_input") == 0)
        return "\nYou did something wrong try again\n";

    else if (strcmp(option, "users_path") == 0)
        return "Users/";

    else if (strcmp(option, "no_such_user") == 0)
        return "\nThere is no such user\n";

    else if (strcmp(option, "next_user") == 0)
        return "\n[E]xit\n[N]ext\n";


    return NULL;
}

size_t recive_message(int socket, char *message) {
    int size = 0;
    switch (size = (int) TEMP_FAILURE_RETRY(recv(socket, message, NMMAX, 0))) {
        case 0:
            return 0;
        case -1:
            ERR("Read from user");
        default:
            return size;
    }

}

void send_message(int socket, char *message) {
    if (TEMP_FAILURE_RETRY(send(socket, message, strlen(message), 0)) == -1)
        ERR("write");
}

void strip(char *s) {
    char *p2 = s;
    while (*s != '\0') {
        if (*s != '\t' && *s != '\n' && *s != '\r' && *s != '\\' && *s != ' ')
            *p2++ = *s++;
        else
            ++s;
    }
    *p2 = '\0';
}


int create_dir(char *path_to_file) {
    if (mkdir(path_to_file, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == -1) {
        if (EEXIST == errno)
            return 1;
        else
            ERR("Creating specific user directory");
    }

    return 0;
}

char *concat_2d_array_to_string(char *twodimarray[], int size_of_content, int num_of_elem, char *labels[],
                                int size_of_labels) {
    char *result;
    char *temp;
    int i = 0;
    int j = 0;

    result = (char *) malloc((size_t) (num_of_elem + size_of_content * 3 + size_of_labels));
    temp = result;
    for (; j < num_of_elem; j++) {

        if (size_of_labels != 0)
            while (*labels[j] != '\0' && (*result++ = *labels[j]++));
        else
            *result++ = (char) (j + '0');
        *result++ = ' ';


        while (i < num_of_elem && *twodimarray[j] != '\0') {
            *result++ = *twodimarray[j]++;
            i++;
        }
        i = 0;
        *result++ = '\n';
    }


    for (int i = 0; i < NUM_OF_PREFERENCES; i++) {
        twodimarray[i] = NULL;
    }
    temp[size_of_content + size_of_labels + 2 * num_of_elem] = '\0';
    return temp;
}


char sublist_letter(char *pstring) {
    char result;

    if (strlen(pstring) < 2)
        return -1;

    result = *++pstring;

    return result;
}

int sublist_number(char *pstring) {
    if (strlen(pstring) < 2)
        return -1;
    int result = 0;
    char *remeinder;
    char *temp;
    pstring++;
    remeinder = (char *) calloc(1, strlen(pstring) - 1);
    temp = remeinder;
    while ((*remeinder++ = *pstring++));

    result = atoi(temp);

    return result;
}


char *cutString(char *fullpath, char fromCrop) {


    char *result;
    size_t i = strlen(fullpath);
    for (; i >= 0; i--) {
        if (fullpath[i] == fromCrop) {
            result = (char *) malloc(i);
            memcpy(result, fullpath, i + 1);
            result[i + 1] = '\0';
            return result;
        }

    }

    return fullpath;
}


int fill_position_char(char chosen, char a, char A, char b, char B) {
    return (chosen == a || chosen == A || chosen == b || chosen == B) ? 1 : 0;

}

int fill_position_number(int chosen, int range_left, int range_right) {
    return (chosen >= range_left && chosen <= range_right) ? 1 : 0;

}

void sethandler(void (*f)(int), int sigNo) {
    struct sigaction act;
    memset(&act, 0x00, sizeof(struct sigaction));
    act.sa_handler = f;

    if (-1 == sigaction(sigNo, &act, NULL))
        perror("sigaction");
}


char *lock_and_read(char *content, char path_to_file[], int *filesize) {

    int size = 0;
    FILE *fp;
    if ((fp = fopen(path_to_file, "r")) == NULL) {
        printf("%s", path_to_file);
        ERR("writing user ");
    }

    while (1) {
        if (ftrylockfile(fp) == 0) {
            if (fp == NULL)
                ERR("Locking file");
            else {


                fseek(fp, 0L, SEEK_END);
                if ((*filesize = (int) ftell(fp)) == -1)
                    ERR("ftell file size");
                rewind(fp);
                size = *filesize;
                printf("%d", size);
                if ((content = calloc(1, *filesize)) == NULL)
                    ERR("calloc");

                fread(content, *filesize, 1, fp);

                funlockfile(fp);

                if (fclose(fp) == EOF)
                    ERR("Closing file");
            }
            break;

        }
    }
    return content;
}


void lock_and_write(char *content, char *path_to_file, char *mode) {


    FILE *fp;
    if ((fp = fopen(path_to_file, mode)) == NULL) {
        printf("%s", path_to_file);
        ERR("writing to user property");

    }


    while (1) {
        if (ftrylockfile(fp) == 0) {
            if (fp == NULL)
                ERR("Locking file");
            else {
                fprintf(fp, "%s", content);
                funlockfile(fp);
                fclose(fp);
            }
            break;
        }
    }
}


int listdir(const char *name, char *my_name, int look_for_his_own, char *directories[]) {
    int i = 0;

    DIR *dir;
    struct dirent *entry;

    if (!(dir = opendir(name)))
        return 0;
    if (!(entry = readdir(dir)))
        return 0;

    do {
        if (entry->d_type == DT_DIR) {
            char path[FD_SETSIZE];
            int len = snprintf(path, sizeof(path) - 1, "%s%s", name, entry->d_name);
            path[len] = 0;
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;

            if (look_for_his_own && strcmp(entry->d_name, my_name) != 0) {
                directories[i] = entry->d_name;
                i++;
            }
            else if (!look_for_his_own) {
                directories[i] = entry->d_name;
                i++;
            }


            // listdir(path, level + 1, directories);
        }

    } while ((entry = readdir(dir)));
    if (closedir(dir) == -1)
        ERR("closing dir");
    dir = NULL;
    free(entry);
    return i;
}

