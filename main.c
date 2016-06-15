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
#include <semaphore.h>
#include "Utilities.h"

#define ERR(source) (perror(source),\
		     fprintf(stderr,"%s:%d\n",__FILE__,__LINE__),\
		     exit(EXIT_FAILURE))

#define BACKLOG 3
#define CHUNKSIZE 500
#define NMMAX 30
#define THREAD_NUM 3
#define FS_NUM 2

#define ERRSTRING "No such file or directory\n"

volatile sig_atomic_t work = 1;

typedef struct
{
    int id;
    int *idlethreads;
    int *socket;
    int *condition;
    pthread_cond_t *cond;
    pthread_mutex_t *mutex;
    sem_t *semaphore;
} thread_arg;


typedef struct {
    char *login;
    int socket;
    char *password;

}User;

void siginthandler(int sig)
{
    work = 0;
}

void usage(char *name)
{
    fprintf(stderr, "USAGE: %s port workdir\n",name);
    exit(EXIT_FAILURE);
}

void sethandler(void (*f)(int), int sigNo)
{
    struct sigaction act;
    memset(&act, 0x00, sizeof(struct sigaction));
    act.sa_handler = f;

    if (-1 == sigaction(sigNo, &act, NULL))
        ERR("sigaction");
}




int make_socket(int domain, int type)
{
    int sock;
    sock = socket(domain, type, 0);
    if (sock < 0)
        ERR("socket");

    return sock;
}

void communicate(int clientfd)
{
    ssize_t size;
    char login_password[NMMAX+1];
   // char buffer[CHUNKSIZE];

    if ((size = TEMP_FAILURE_RETRY(recv(clientfd, login_password, NMMAX + 1, MSG_WAITALL))) == -1)
        ERR("read");

    login_password[size] = '\0';

    printf("%s", login_password);


}

void cleanup(void *arg)
{
    pthread_mutex_unlock((pthread_mutex_t *)arg);
}

void *threadfunc(void *arg)
{
    char ** result;
    char  response_message[NMMAX +1];
   // int clientfd;
    thread_arg targ;
    ssize_t size;
    User *args= (User*) arg;
    memcpy(&targ, arg, sizeof(targ));


    while(work)
    {
        if((size = TEMP_FAILURE_RETRY(recv(args->socket, response_message, NMMAX + 1, MSG_WAITALL))) == 0)
            break;

        response_message[size] = '\0';

        printf("%s",response_message);
        result = str_split(response_message,' ');

        if(sizeof(result)/sizeof(result[0]) > 2)
            printf("Elo");
        else
            printf("Mordeczki");

    }

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

    char * askforlogin;
    if ((nfd = TEMP_FAILURE_RETRY(accept(sfd, NULL, NULL))) < 0)
    {
        if (EAGAIN == errno || EWOULDBLOCK == errno)
            return -1;
        ERR("accept");
    }

    askforlogin = "Write your login and password divided by space: \n";

    if (TEMP_FAILURE_RETRY(send(nfd, askforlogin, strlen(askforlogin), 0)) == -1)
        ERR("write");



    return nfd;
}

void dowork(int socket)
{
    int clientfd;
    sigset_t mask, oldmask;
    pthread_t thread;
    User *userArg;
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

            userArg->socket = clientfd;

            if (pthread_create(&thread, NULL,threadfunc, (void *)userArg) != 0) perror("Pthread_create");
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

void pcleanup(sem_t *semaphore, pthread_mutex_t *mutex, pthread_cond_t *cond)
{
    if (sem_destroy(semaphore) != 0)
        ERR("sem_destroy");
    if (pthread_mutex_destroy(mutex) != 0)
        ERR("pthread_mutex_destroy");
    if (pthread_cond_destroy(cond) != 0)
        ERR("pthread_cond_destroy");
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


    dowork(socket);


    if (TEMP_FAILURE_RETRY(close(socket)) < 0)
        ERR("close");
    return EXIT_SUCCESS;
}