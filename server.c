
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <pthread.h>
#include "Utilities.h"
#include "message_flow.h"
#include "structures.h"


#define BACKLOG 3
#define NMMAX 30

volatile sig_atomic_t work = 1;




void siginthandler(int sig)
{
    work = 0;
}

void usage(char *name)
{
    fprintf(stderr, "USAGE: %s port workdir\n",name);
    exit(EXIT_FAILURE);
}




int make_socket(int domain, int type)
{
    int sock;
    sock = socket(domain, type, 0);
    if (sock < 0)
        ERR("socket");

    return sock;
}




void cleanup(void *arg)
{
    pthread_mutex_unlock((pthread_mutex_t *)arg);
}

void *threadfunc(void *arg) {


    int existing_user = 0;
    char response_message[NMMAX];
    int communication_progress = 0;
    char * filepath;
    size_t size;
    ThreadArg *args = (ThreadArg *) arg;

    filepath = (char*)malloc(strlen(args->filePath));


    strcpy(filepath,args->filePath);

    while (work) {
        if ((size = TEMP_FAILURE_RETRY(recv(args->socket, &response_message, NMMAX - 1, 0))) == 0)
            break;

        strip(response_message);
        response_message[size - 2] = '\0';

        communication_flow(&communication_progress, args, size, response_message, &existing_user,filepath);
    }

    free(args->user_info->profile);
    free(args->user_info);
    free(args);
    return NULL;
}





int bind_tcp_socket(uint16_t port)
{
    struct sockaddr_in addr;
    int socketfd, t=1;

    socketfd = make_socket(PF_INET, SOCK_STREAM);
    memset(&addr, 0x00, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &t, sizeof(t)))
        ERR("setsockopt");
    if (bind(socketfd, (struct sockaddr *) &addr, sizeof(addr)) < 0)
        ERR("bind");
    if (listen(socketfd, BACKLOG) < 0)
        ERR("listen");

    return socketfd;
}

int add_new_client(int sfd)
{
    int nfd;
    char * askforlogin = "Write your login:\n";

    if ((nfd = TEMP_FAILURE_RETRY(accept(sfd, NULL, NULL))) < 0)
    {
        if (EAGAIN == errno || EWOULDBLOCK == errno)
            return -1;
        ERR("accept");
    }



    if (TEMP_FAILURE_RETRY(send(nfd, askforlogin, strlen(askforlogin), 0)) == -1)
        ERR("write");



    return nfd;
}

void dowork(int socket, char *filepath)
{

    int clientfd;
    sigset_t mask, oldmask;
    pthread_t thread;
    User *userArg;
    Profile *profile;

    ThreadArg *threadArgHead = NULL;

    fd_set base_rfds, rfds;
    FD_ZERO(&base_rfds);
    FD_SET(socket, &base_rfds);
    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    sigprocmask(SIG_BLOCK, &mask, &oldmask);
    while (work)
    {
        rfds = base_rfds;
        if (pselect(socket + 1, &rfds, NULL, NULL, NULL, &oldmask) > 0)
        {
            if ((clientfd = add_new_client(socket)) == -1)
                continue;

            if((userArg=(User*)malloc(sizeof(User)))==NULL)
                perror("Malloc:");

            if((threadArgHead=(ThreadArg*)malloc(sizeof(ThreadArg)))==NULL)
                perror("Malloc:");

            if((threadArgHead->filePath =(char*)malloc(sizeof(filepath)))==NULL)
                perror("Malloc:");

            if((profile =(Profile*)malloc(sizeof(Profile)))==NULL)
                perror("Malloc:");

            threadArgHead->user_info = userArg;
            threadArgHead->user_info->profile = profile;
            threadArgHead->socket = clientfd;
            threadArgHead->filePath =filepath;


            if (pthread_create(&thread, NULL,threadfunc, (void *)threadArgHead) != 0) perror("Pthread_create");
            if (pthread_detach(thread) != 0) perror("Pthread_detach");


        }
        else
        {
            if (EINTR == errno)
                continue;
            ERR("pselect");
        }
    }
}


int main(int argc, char **argv)
{


    int socket, new_flags;
    char * filePath;

    if((atoi(argv[1]) == 0) || (atoi(argv[1]) < 1024 || atoi(argv[1]) > 65535))
    {
        usage(argv[0]);
    }


    sethandler(SIG_IGN, SIGPIPE);
    sethandler(siginthandler, SIGINT);

    socket=bind_tcp_socket(atoi(argv[1]));
    new_flags = fcntl(socket, F_GETFL) | O_NONBLOCK;
    fcntl(socket, F_SETFL, new_flags);

    filePath = CutFileName(__FILE__, '/');
    dowork(socket, filePath);


    if (TEMP_FAILURE_RETRY(close(socket)) < 0)
        ERR("close");
    return EXIT_SUCCESS;
}