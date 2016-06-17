//
// Created by username on 15.06.16.
//


#define _GNU_SOURCE
#ifndef STRUCTURES_H
#define STRUCTURES_H



#define ERR(source) (perror(source),\
		     fprintf(stderr,"%s:%d\n",__FILE__,__LINE__),\
		     exit(EXIT_FAILURE))

typedef struct {
    char sex;
    int weight;
    int age;
    char * sexual_preferences;
    char* hair_color;


}Profile;

typedef struct {
    char *login;
    char *password;
    Profile *profile;

}User;

typedef struct{
    char *filePath;
    int socket;
    User *user_info;

	struct ThreadArg *next;

}ThreadArg;

#endif