#include "epoll.h"

int txFile(int socketFD, char *buffer, int length) {


    //probably should see if the file is there first
    int pid = fork();
    switch(pid) {
    case 0:

        write(1, buffer, length);
        printf("\n");
        exit(EXIT_SUCCESS);
    case -1:
        perror("Fork");
        abort();
        return 1;
    default:
        if (buffer != NULL) {
            free(buffer);
        }
        return 0;
    }
    //TODO make new process to find file that matches socket data and then transmit it in its on process
    return 0;
}

int main (int argc, char *argv[]) {
    int socketFD;
    int epollFD;
    struct epoll_event *events;
    int (*fnPtr)(int, char*, int) = txFile;



    if (argc != 2) {
        fprintf (stderr, "Usage: %s [port]\n", argv[0]);
        exit (EXIT_FAILURE);
    }

    socketFD = validateSocket(argv[1]);
    bindandListenSocket(socketFD);
    epollFD = createEPoll();
    setEPollSocket(epollFD, socketFD, &events);

    eventLoop(socketFD, epollFD, events, fnPtr);

    close (socketFD);
    return EXIT_SUCCESS;
}
