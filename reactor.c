

#include "reactor.h"

// Add a new file descriptor to the set
void add_to_pfds(p_Reactor reactor, int newfd) {
    // If we don't have room, add more space in the pfds array
    if (reactor->p_HashMap->size >= reactor->p_HashMap->capacity) {
        reactor->pfds = realloc(reactor->pfds, sizeof(*reactor->pfds) * (reactor->p_HashMap->capacity * 2));
    }

    (reactor->pfds)[reactor->p_HashMap->size].fd = newfd;
    (reactor->pfds)[reactor->p_HashMap->size].events = POLLIN; // Check ready-to-read
}

// Remove an index from the set
void del_from_pfds(struct pollfd pfds[], int i, int *fd_count) {
    // Copy the one from the end over this one
    pfds[i] = pfds[*fd_count - 1];

    (*fd_count)--;
}
void accept_fun(int values, p_Reactor reactor,int listener) {
    // If listener is ready to read, handle new connection

    struct sockaddr_storage remoteaddr; // Client address
    socklen_t addrlen;

    addrlen = sizeof remoteaddr;
    int newfd = accept(listener,
                   (struct sockaddr *)&remoteaddr,
                   &addrlen);

    if (newfd == -1) {
        perror("accept");
    } else {
        printf("accepted fd: %d\n",newfd);

        addFd(reactor, newfd, (handler_t) print_fun);

    }
}


void print_fun(int values, p_Reactor reactor,int fd) {
    char buf[1024];    // Buffer for client data
    bzero(buf,sizeof buf);
    // If not the listener, we're just a regular client
    int nbytes = recv(fd, buf, sizeof buf, 0);

    if (nbytes <= 0) {
        // Got error or connection closed by client
        if (nbytes == 0) {
            // Connection closed
            printf("pollserver: socket %d hung up\n", fd);
            removeFd(reactor,fd);
        } else {
            perror("recv");
        }
        close(fd); // Bye!
        removeFd(reactor,fd);

    } else {
        printf("fd: %d, send: %s",fd,buf);
    }

}

// Function executed by the thread

void *main_function(void *this) {
    p_Reactor reactor = (p_Reactor) this;

// Main loop
    for (;;) {
        int poll_count = poll(reactor->pfds, reactor->p_HashMap->size, -1);
        if (poll_count == -1) {
            perror("poll");
            exit(1);
        }
        // Run through the existing connections looking for data to read
        for (int i = 0; i < reactor->p_HashMap->size; i++) {
            // Check if someone's ready to read
            if (reactor->pfds[i].revents & POLLIN) { // We got one!!
                handler_t event1_handler = hashmap_get(reactor->p_HashMap, reactor->pfds[i].fd);
                if (event1_handler != NULL) {
                    event1_handler(2,reactor,reactor->pfds[i].fd);
                }
            }
        } // END for(;;)--and you thought it would never end!
    }
//    return 0;
}

//create reactor, return pointer to struct of reactor
//upon creation is not active only ds are init and alloc
void *createReactor() {
    p_Reactor reactor = (p_Reactor) malloc(sizeof(Reactor));
    reactor->p_HashMap = hashmap_create(START_SIZE);
    reactor->pfds = malloc(sizeof *reactor->pfds * START_SIZE);
    reactor->thread_active = FALSE;
    return reactor;
};

//stop reactor if active
void stopReactor(void *this) {
    p_Reactor reactor = (p_Reactor) this;
    if (!reactor->thread_active) return;

    if (pthread_cancel(&(reactor->thread_id) != 0)) {
        printf("Failed to stop thread.\n");
    }
};

//start thread of reactor, will live in busy loop and call poll
void startReactor(void *this) {
    p_Reactor reactor = (p_Reactor) this;
    if (pthread_create(&(reactor->thread_id), NULL, main_function, reactor) != 0) {
        printf("Failed to create thread.\n");
        return;
    }
    reactor->thread_active = TRUE;
};

//handler is main_function who called when fd is "hot"
void addFd(void *this, int fd, handler_t handler) {
    p_Reactor reactor = (p_Reactor) this;
    add_to_pfds(reactor, fd);
    hashmap_insert(reactor->p_HashMap, fd, handler);
};

void removeFd(void *this, int fd) {
    p_Reactor reactor = (p_Reactor) this;
    del_from_pfds(reactor->pfds, fd, &reactor->p_HashMap->size);
    hashmap_delete(reactor->p_HashMap, fd);
};

//wait on pthread_join(3) until tread of reactor will finish
void WaitFor(void *this) {
    p_Reactor reactor = (p_Reactor) this;
    if (!reactor->thread_active) return;
    pthread_join(reactor->thread_id, NULL);
};

