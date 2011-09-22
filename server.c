#include "epoll.h"

typedef struct communicationPacket {
    int packetlength;
    char getSend;
    long fileSize;
    char fileName[FILENAME_MAX];
} CPACKET, *PCPACKET;

void rxFile(int socketFD, char *buffer, int length) {
    PCPACKET pkt = (PCPACKET) buffer;
    printf("%c\n", pkt->packetlength);
}

void txFile(int socketFD, char *buffer, int length) {
    printf("txfile\n");
}

int parseClientRequest(int socketFD, char *buffer, int length) {

    //probably should see if the file is there first
    int pid = fork();

    switch(pid) {
    case 0:
        if (buffer[0] == 's') {
            txFile(socketFD, buffer, length);
        } else if (buffer[0] == 'g') {
            rxFile(socketFD, buffer, length);
        } else {
            //error condition
        }
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
    return 0;
}

int main (int argc, char *argv[]) {
    int socketFD;
    int epollFD;
    struct epoll_event *events;
    int (*fnPtr)(int, char*, int) = parseClientRequest;



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
