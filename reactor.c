

#include "reactor.h"


void accept_fun(int values, p_Reactor reactor,int listener) {
    // If listener is ready to read, handle new connection

    struct sockaddr_storage sockaddrStorage; // Client address
    socklen_t socklen;

    socklen = sizeof sockaddrStorage;
    int newFd = accept(listener,
                       (struct sockaddr *)&sockaddrStorage,
                       &socklen);

    if (newFd == -1) {
        perror("[-] accept accept");
        exit(1);
    } else {
        printf("[+] accepted fd: %d\n", newFd);
        addFd(reactor, newFd, (handler_t) print_fun);

    }
}


void print_fun(int values, p_Reactor reactor,int fd) {
    char buf[256];    // ass beej
    bzero(buf,sizeof buf); // so dont get trash

    // If not the listener, we're just a regular client
    size_t bytesReceived = recv(fd, buf, sizeof buf, 0);

    if (bytesReceived <= 0) {
        close(fd); // Bye!
        removeFd(reactor,fd);//add free
        // Got error or connection closed by client
        if (bytesReceived == 0) {
            // Connection closed
            printf("[!] socket %d hung up\n", fd);
        } else {
            perror("recv massage");
            exit(1);
        }

    } else {
        printf("( %d ): %s",fd,buf);
    }

}

// Function executed by the thread
void *main_function(void *this) {
    p_Reactor reactor = (p_Reactor) this;

// Main loop
    while (reactor->thread_active) {
        int poll_count = poll(reactor->pfds, reactor->p_HashMap->size, -1);
        if (poll_count == -1) {
            perror("poll");
            exit(1);
        }
        // Run through the existing connections looking for data to read
        for (int i = 0; i < reactor->p_HashMap->size; i++) {
            // Check if someone's ready to read
            if (reactor->pfds[i].revents & POLLIN) { // We got one!!
                //get assign function
                handler_t event1_handler = hashmap_get(reactor->p_HashMap, reactor->pfds[i].fd);
                if (event1_handler != NULL) {
                    event1_handler(2,reactor,reactor->pfds[i].fd);
                }
            }
        } // END for(;;)--and you thought it would never end!
    }
    return 0;
}

//create reactor, return pointer to struct of reactor
//upon creation is not active only ds are init and alloc
void *createReactor() {
    p_Reactor reactor = (p_Reactor) malloc(sizeof(Reactor));
    //if failed
    if(reactor == NULL){
        return NULL;
    }
    reactor->p_HashMap = hashmap_create(START_SIZE);
    reactor->pfds = malloc(sizeof *reactor->pfds * START_SIZE);
    reactor->thread_active = FALSE;
    return reactor;
};



//start thread of reactor, will live in busy loop and call poll
void startReactor(void *this) {
    if(this == NULL)return;
    p_Reactor reactor = (p_Reactor) this;
    if (pthread_create(&(reactor->thread_id), NULL, main_function, reactor) != 0) {
        printf("Failed to create thread.\n");
        return;
    }
    reactor->thread_active = TRUE;
};

//stop reactor if active
void stopReactor(void *this) {
    if(this == NULL)return;

    p_Reactor reactor = (p_Reactor) this;
    if (!reactor->thread_active) return;

    //stop loop and wait for him
    reactor->thread_active = FALSE;
    pthread_join(reactor->thread_id,NULL);

};

//handler is main_function who called when fd is "hot"
void addFd(void *this, int fd, handler_t handler) {
    if(this == NULL)return;
    p_Reactor reactor = (p_Reactor) this;
    // If we don't have room, add more space in the pfds array
    if (reactor->p_HashMap->size >= reactor->p_HashMap->capacity) {
        reactor->pfds = realloc(reactor->pfds, sizeof(*reactor->pfds) * (reactor->p_HashMap->capacity * 2));
    }

    (reactor->pfds)[reactor->p_HashMap->size].fd = fd;
    (reactor->pfds)[reactor->p_HashMap->size].events = POLLIN; // Check ready-to-read
    hashmap_insert(reactor->p_HashMap, fd, handler);
};

void removeFd(void *this, int fd) {
    if(this == NULL)return;

    p_Reactor reactor = (p_Reactor) this;
    int i;
    for (i = 0; i < reactor->p_HashMap->size && reactor->pfds[i].fd != fd; ++i);
    reactor->pfds[i] = reactor->pfds[reactor->p_HashMap->size-1];
    hashmap_delete(reactor->p_HashMap, fd);
};

//wait on pthread_join(3) until tread of reactor will finish
void WaitFor(void *this) {
    if(this == NULL)return;

    p_Reactor reactor = (p_Reactor) this;
    if (!reactor->thread_active) return;
    pthread_join(reactor->thread_id, NULL);
};

void freeAll(void *this){
    if(this == NULL)return;
    p_Reactor reactor = (p_Reactor) this;

    stopReactor(reactor);

    for (int i = 0; i < reactor->p_HashMap->size; i++) {
        int fd = reactor->pfds[i].fd;
        close(fd);
        removeFd(reactor, fd);
    }
    free(reactor->pfds);
    free(reactor->p_HashMap->buckets);
    free(reactor->p_HashMap);
    free(reactor);
}

