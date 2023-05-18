#ifndef REACTOR_H
#define REACTOR_H

#include <stdio.h>
#include <pthread.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>

#include "HashMap.h"

#define PORT "9034"   // Port we're listening on

typedef struct Reactor {
    pthread_t thread_id;
    HashMap * p_HashMap;
    struct pollfd *pfds;
    int thread_active;
} Reactor,*p_Reactor;

//create reactor, return pointer to struct of reactor
//upon creation is not active only ds are init and alloc
void *createReactor();

//stop reactor if active
void stopReactor(void * this);

//start thread of reactor, will live in busy loop and call poll
void startReactor(void * this);

//handler is main_function who called when fd is "hot"
void addFd(void * this, int fd,handler_t handler);

//remove fd from hashmap
void removeFd(void *this, int fd);

//wait on pthread_join(3) until tread of reactor will finish
void WaitFor(void * this);

void print_fun(int , p_Reactor ,int );
void accept_fun(int , p_Reactor ,int );
#endif /* REACTOR_H */


