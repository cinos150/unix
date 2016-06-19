
#ifndef STRUCTURES_H
#define STRUCTURES_H



#define ERR(source) (perror(source),\
		     fprintf(stderr,"%s:%d\n",__FILE__,__LINE__),\
		     exit(EXIT_FAILURE))


typedef struct thread_args{
    int socket;
	char * filepath;
	char *login;
	struct thread_args *next;

}ThreadArg;

#endif