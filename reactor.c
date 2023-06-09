#include "reactor.h"


void accept_fun(int values, p_Reactor reactor, int listener) {
    // If listener is ready to read, handle new connection

    struct sockaddr_storage sockaddrStorage; // Client address
    socklen_t socklen;

    socklen = sizeof sockaddrStorage;
    int newFd = accept(listener,
                       (struct sockaddr *) &sockaddrStorage,
                       &socklen);

    if (newFd == -1) {
        perror("[-] accept accept");
        exit(1);
    } else {
        printf("[+] accepted fd: %d\n", newFd);
        addFd(reactor, newFd, (handler_t) chat_fun);

    }
}

void chat_fun(int values, p_Reactor reactor, int fd) {

    char buf[256];    // as beej
    bzero(buf, sizeof buf); // so dont get trash

    // If not the listener, we're just a regular client
    size_t bytesReceived = recv(fd, buf, sizeof buf, 0);

    if (bytesReceived <= 0) {
        close(fd); // Bye!
        removeFd(reactor, fd);//add free
        // Got error or connection closed by client
        if (bytesReceived == 0) {
            // Connection closed
            printf("[!] socket %d hung up\n", fd);
        } else {
            perror("recv massage");
            exit(1);
        }

    } else {
        //got data

        // Send to everyone!
        for (int j = 0; j < reactor->p_HashMap->size; j++) {
            int dest_fd = reactor->pfds[j].fd;

            // only to other that have chat func (exclude listener) exclude ourselves
            if (hashmap_get(reactor->p_HashMap, dest_fd) == (handler_t) chat_fun && dest_fd != fd) {
                if (send(dest_fd, buf, bytesReceived, 0) == -1) {
                    perror("send");
                }
            }
        }
        //print what he sends
        printf("( %d ): %s", fd, buf);

    }

}

// Function executed by the thread
void *main_function(void *this) {
    printf("============= Reactor thread =============\n");

    p_Reactor reactor = (p_Reactor) this;

// Main loop
    int current_size = reactor->p_HashMap->size;

    while (reactor->thread_active) {
        int poll_count = poll(reactor->pfds, current_size, -1);
        if (poll_count == -1) {
            perror("poll");
            exit(1);
        }
        // Run through the existing connections looking for data to read
        for (int i = 0; i < current_size; i++) {
            // Check if someone's ready to read
            if (reactor->pfds[i].revents & POLLIN) { // We got one!!
                //get assign function
                handler_t handler = hashmap_get(reactor->p_HashMap, reactor->pfds[i].fd);
                if (handler != NULL) {
                    handler(2, reactor, reactor->pfds[i].fd);
                }
            }
        }
        current_size = reactor->p_HashMap->size;
    }// END for(;;)--and you thought it would never end!
    return 0;
}

//create reactor, return pointer to struct of reactor
//upon creation is not active only ds are init and alloc
void *createReactor() {
    p_Reactor reactor = (p_Reactor) malloc(sizeof(Reactor));
    //if failed
    if (reactor == NULL) {
        return NULL;
    }
    reactor->p_HashMap = hashmap_create(START_SIZE);
    reactor->pfds = malloc(sizeof *reactor->pfds * START_SIZE);
    reactor->thread_active = FALSE;

    printf("[+] Reactor created\n");

    return reactor;
}


//start thread of reactor, will live in busy loop and call poll
void startReactor(void *this) {
    if (this == NULL)return;
    p_Reactor reactor = (p_Reactor) this;
    if (pthread_create(&(reactor->thread_id), NULL, main_function, reactor) != 0) {
        printf("Failed to create thread.\n");
        return;
    }
    reactor->thread_active = TRUE;
    printf("[+] Reactor started\n");

}

//stop reactor if active
void stopReactor(void *this) {
    if (this == NULL)return;

    p_Reactor reactor = (p_Reactor) this;
    if (!reactor->thread_active) return;

    //stop loop and wait for him
    reactor->thread_active = FALSE;
    pthread_join(reactor->thread_id, NULL);
    printf("[+] Reactor stopped\n");

}

//handler is main_function who called when fd is "hot"
void addFd(void *this, int fd, handler_t handler) {
    if (this == NULL)return;
    p_Reactor reactor = (p_Reactor) this;
    // If we don't have room, add more space in the pfds array
    if (reactor->p_HashMap->size >= reactor->p_HashMap->capacity) {
        reactor->pfds = realloc(reactor->pfds, sizeof(*reactor->pfds) * (reactor->p_HashMap->capacity * 2));
        if (reactor->pfds == NULL)
            exit(ERROR);
    }

    (reactor->pfds)[reactor->p_HashMap->size].fd = fd;
    (reactor->pfds)[reactor->p_HashMap->size].events = POLLIN; // Check ready-to-read
    hashmap_insert(reactor->p_HashMap, fd, handler);
}

void removeFd(void *this, int fd) {
    if (this == NULL)return;

    p_Reactor reactor = (p_Reactor) this;
    int i;
    for (i = 0; i < reactor->p_HashMap->size && reactor->pfds[i].fd != fd; ++i);
    reactor->pfds[i] = reactor->pfds[reactor->p_HashMap->size - 1];
    hashmap_delete(reactor->p_HashMap, fd);
}

//wait on pthread_join(3) until tread of reactor will finish
void WaitFor(void *this) {
    if (this == NULL)return;

    p_Reactor reactor = (p_Reactor) this;
    if (!reactor->thread_active) return;
    pthread_join(reactor->thread_id, NULL);
}

void freeAll(void *this) {
    if (this == NULL)return;
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

