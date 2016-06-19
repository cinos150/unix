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
#include "LinkedList.h"


#define BACKLOG 3
#define NMMAX 30

volatile sig_atomic_t work = 1;




void siginthandler(int sig)
{
    work = 0;
}

void usage(char *name)
{
    fprintf(stderr, "USAGE: %s port\n",name);
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
    size_t size;
    ThreadArg *args = (ThreadArg *) arg;

    char directory_path[strlen(args->filepath)+1];

    snprintf(directory_path,sizeof(directory_path),"%s",args->filepath);


    while (work) {

        directory_path[strlen(args->filepath)] = '\0';

        if((size = recive_message(args->socket,response_message)) ==0)
            break;



        strip(response_message);
        response_message[size - 2] = '\0';

        if(strlen(response_message)<1)
        {
            send_message(args->socket,"no empty inputs please, try again\n");
            continue;
        }

       communication_flow(&communication_progress, args, size, response_message, &existing_user,directory_path);

    }


    delete_from_list(args->login);



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
    char * askforlogin = "Write your login (spaces will be ignored):\n";

    if ((nfd = (int) TEMP_FAILURE_RETRY(accept(sfd, NULL, NULL))) < 0)
    {
        if (EAGAIN == errno || EWOULDBLOCK == errno)
            return -1;
        ERR("accept");
    }



    send_message(nfd,askforlogin);

    return nfd;
}

void dowork(int socket, char *filepath)
{

    int clientfd;
    sigset_t mask, oldmask;
    pthread_t thread;



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
            threadArgHead =  add_to_list(clientfd,cutString(__FILE__,'/'));
           // threadArgHead =  add_to_list(clientfd,filepath);


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

    if((atoi(argv[1]) == 0) || (atoi(argv[1]) < 1024 || atoi(argv[1]) > 65535))
    {
        usage(argv[0]);
    }


    sethandler(SIG_IGN, SIGPIPE);
    sethandler(siginthandler, SIGINT);

    socket=bind_tcp_socket(atoi(argv[1]));
    new_flags = fcntl(socket, F_GETFL) | O_NONBLOCK;
    fcntl(socket, F_SETFL, new_flags);


    dowork(socket,argv[0]);


    if (TEMP_FAILURE_RETRY(close(socket)) < 0)
        ERR("close");
    return EXIT_SUCCESS;
}