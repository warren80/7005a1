#include "epoll.h"
#include "server.h"

int fileSize(FILE * pFile) {
    int fd;
    struct stat buf;

    fd = fileno(pFile); //if you have a stream (e.g. from fopen), not a file descriptor.
    if (fstat(fd, &buf) == -1) {
        perror("fstat");
        abort();
    }
    return buf.st_size;
}

void writeFileToSocket(FILE* pFile, int socketFD) {
    char buf[MAXBUFFSIZE];
    int i = 0;
    while((buf[i++] = getc(pFile)) != EOF) {
        if (i == MAXBUFFSIZE + 1) {
            write(socketFD, buf, MAXBUFFSIZE);
            i = 0;
        }
    }
    pclose(pFile);
    write(socketFD, buf, i-1);
}



void txFile(int socketFD, char *buffer, int length) {
    PCPACKET pkt = (PCPACKET) buffer;
    printf("%c\n", pkt->packetlength);
}

void rxFile(int socketFD, char *buffer, int length) {
    PCPACKET pkt = (PCPACKET) buffer;
    int clientSocketFD;
    char filename[FILENAME_MAX];
    snprintf(filename, 15 ,"files/%s", pkt->fileName);
    filename[15] = 0;

    FILE * pFile = fopen(filename, "r");
    if (pFile == NULL) {
        perror("fopen");
        abort();
    }

    clientSocketFD = getClientSocket(socketFD);
    pkt->totalPackets =
    writeFileToSocket(pFile,clientSocketFD);
    close(clientSocketFD);
}

int getClientSocket(int socketFD) {
    int sd;
    socklen_t socketLength;
    struct sockaddr_in addr_in;
    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Cannot create socket");
        exit(1);
    }
    socketLength = sizeof(addr_in);
    getpeername(socketFD, (struct sockaddr*)&addr_in, &socketLength);
    addr_in.sin_port = htons(CLIENTPORT);
    if (connect (sd, (struct sockaddr *)&addr_in, socketLength) == -1) {
        fprintf(stderr, "Can't connect to server\n");
        perror("connect");
        exit(1);
    }
    return sd;
}

void listFiles(int socketFD, char *buffer, int length) {
    FILE* pFile = popen("ls files -l", "r");
    if (pFile == NULL) {
        exit(EXIT_FAILURE);
    }
    writeFileToSocket(pFile, getClientSocket(socketFD));
}

int parseClientRequest(int socketFD, char *buffer, int length) {
    int pid = fork();
    switch(pid) {
    case 0:
        if (buffer[0] == TXMSG) {
            txFile(socketFD, buffer, length);
        } else if (buffer[0] == RXMSG) {
            rxFile(socketFD, buffer, length);
        } else if (buffer[0] == LIST) {
            listFiles(socketFD, buffer, length);
        } else {
            printf("error\n");
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

int main(int argc, char *argv[]) {
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
    close(socketFD);
    return EXIT_SUCCESS;
}
