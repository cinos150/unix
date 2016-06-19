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

size_t recive_message(int socket, char *message)
{
    int size =0;
    switch (size = (int) TEMP_FAILURE_RETRY(recv(socket, message, NMMAX, 0)))
    {
        case 0:
            return 0;
        case -1:
            ERR("Read from user");
        default:
            return size;
    }

}

void send_message(int socket, char* message)
{
    if (TEMP_FAILURE_RETRY(send(socket, message, strlen(message), 0)) == -1)
        ERR("write");
}

void strip(char *s)
{
    char *p2 = s;
    while(*s != '\0')
    {
        if(*s != '\t' && *s != '\n' && *s != '\r' && *s != '\\' && *s !=' ')
            *p2++=*s++;
        else
            ++s;
    }
    *p2 ='\0';
}



char * read_all_from_file(FILE *pFile, char path_to_file[] )
{
    int filesize =0;
    char *filecontent = NULL;
    pFile = fopen(path_to_file, "r");
    if (pFile == NULL) {
        ERR("reading file");
    }
    fseek(pFile, 0L, SEEK_END);
    filesize = (int) ftell(pFile);
    rewind(pFile);



    if ((filecontent = (char *)malloc( (size_t) (filesize + 1))) == NULL)
        perror("Malloc:");



    fscanf(pFile, "%s", filecontent);
    fclose(pFile);

    return filecontent;
}

int create_dir(char *path_to_file)
{
    if (mkdir(path_to_file, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == -1) {
        if (EEXIST == errno)
            return 1;
        else
            ERR("Creating specific user directory");
    }

    return 0;
}

char * concat_2d_array_to_string(char *twodimarray[], int size_of_content,int num_of_elem, char *labels[] , int size_of_labels)
{
    char  *result;
    char *temp;
    int i =0;
    int j =0;

    result = (char*)malloc((size_t) (num_of_elem + size_of_content*3 + size_of_labels));
    temp = result;
    for( ;  j < num_of_elem ;j++) {

        if(size_of_labels !=0 )
            while(*labels[j] != '\0' &&( *result++ = *labels[j]++));
        else
            *result++ = (char) (j + '0');
        *result++ = ' ';



        while(i < num_of_elem && *twodimarray[j]!= '\0')
        {
            *result++ = *twodimarray[j]++;
            i++;
        }
        i =0;
        *result++ = '\n';
    }


    temp[ size_of_content +size_of_labels+ 2* num_of_elem] = '\0';
    return temp;
}


char  sublist_letter(char* pstring)
{
    char result;

    if(strlen(pstring) < 2)
        return -1;

    result = *++pstring;

    return result;
}

int  sublist_number(char* pstring)
{
    if(strlen(pstring) < 2)
        return -1;
    int result =0;
    char *remeinder;
    char * temp;
    pstring++;
    remeinder = (char*)calloc(1,strlen(pstring) -1);
    temp = remeinder;
    while((*remeinder++ = *pstring++));

        result = atoi(temp);

    return result;
}


char * cutString(char *fullpath, char fromCrop)
{



    char *result;
    size_t i =strlen(fullpath);
    for(;i>=0;i--)
    {
        if(fullpath[i] == fromCrop)
        {
            result = (char*)malloc(i);
            memcpy(result,fullpath,i+1);
            result[i+1] = '\0';
            return result;
        }

    }

    return fullpath;
}


int fill_position_char(char chosen, char a, char A, char b, char B)
{
    return (chosen == a ||chosen == A ||chosen == b ||chosen == B) ? 1:0;

}

int fill_position_number(int chosen, int range_left, int range_right)
{
    return (chosen >=range_left && chosen <=range_right)?1:0;

}

void sethandler(void (*f)(int), int sigNo)
{
    struct sigaction act;
    memset(&act, 0x00, sizeof(struct sigaction));
    act.sa_handler = f;

    if (-1 == sigaction(sigNo, &act, NULL))
        perror("sigaction");
}


int  listdir(const char *name, int level, char *directories[])
{
    int i =0;

    DIR *dir;
    struct dirent *entry;

    if (!(dir = opendir(name)))
        return 0;
    if (!(entry = readdir(dir)))
        return 0;

    do {
        if (entry->d_type == DT_DIR) {
            char path[1024];
            int len = snprintf(path, sizeof(path)-1, "%s%s", name, entry->d_name);
            path[len] = 0;
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;

            directories[i] = entry->d_name;
            i++;

            listdir(path, level + 1, directories);
        }

    } while ((entry = readdir(dir)));
    closedir(dir);
    dir = NULL;
    free(entry);
    return i;
}
