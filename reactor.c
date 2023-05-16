#include <stdio.h>

//TODO
typedef struct {
    int member1;
    char member2;
} handler_t;

//create reactor, return pointer to struct of reactor
//upon creation is not active only ds are init and alloc
void *createReactor(){ return NULL; };

//stop reactor if active
void stopReactor(void * this){};

//start thread of reactor, will live in busy loop and call poll
void startReactor(void * this){};

//handler is function who called when fd is "hot"
void addFd(void * this, int fd,handler_t handler){};

//wait on pthread_join(3) until tread of reactor will finish
void WaitFor(void * this){};
