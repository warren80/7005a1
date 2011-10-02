#include "epoll.h"
#include "server.h"

char* readAllDataFromSocket(int socketFD) {
    char * buf;
    buf = malloc(FILENAME_MAX);
    size_t length, packetLength, count;
    length = 0;

    packetLength = read(socketFD, (char *) &length, sizeof(int));

    if (packetLength != sizeof(int)) {
        perror("read");
        exit(1);
    }

    while (1) {
        count = read(socketFD, &buf[length], MAXBUFFSIZE - length);
        length += count;
        if (length == packetLength) {
            return buf;
        }
        if (count == -1) {
            if (errno != EAGAIN) {
                perror ("read");
                return 0;
            }
        } else if(count == packetLength) {
            return buf;
        }
    }
}

FILE* openFile(char* filename, char * access) {
    char file[FILENAME_MAX];
    snprintf(file, strlen(filename) + 6  ,"files/%s", filename);
    FILE * pFile = fopen(file, access);
    if (pFile == NULL) {
        perror("fopen");
        abort();
    }
    return pFile;
}

void txFile(int socketFD, PCPKT packet) {
    socketFD = getClientSocket(socketFD);
    char fileAccess[2];
    fileAccess[0] = 'r';
    fileAccess[1] = '\0';
    FILE * pFile = openFile(packet->filename, fileAccess);
    writeFileToSocket(pFile, socketFD);
}

void rxFile(int socketFD, PCPKT packet) {
    /*
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
    //pkt->totalPackets =
    writeFileToSocket(pFile,clientSocketFD);
    close(clientSocketFD);
    */
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
        fprintf(stderr, "Can't connect to client\n");
        perror("connect");
        exit(1);
    }
    return sd;
}

void listFiles(int socketFD) {
    PFTPKT packet = malloc(sizeof(FTPKT));
    int c = 0, i = 0, x =0;
    FILE* pFile = popen("ls files", "r");
    if (pFile == NULL) {
        exit(EXIT_FAILURE);
    }
    while(1){
        c = getc(pFile);
        if(c == EOF){ break; }
        x++;
        packet->data[i++] = c;
    }
    packet->pl = i; // packet length is date + 2 unsigned ints.
    packet->packetNum = 0;
    write(socketFD,(void *) packet, packet->pl + (2*sizeof(packet->pl)));
    
}

int parseClientRequest(int socketFD, char * buffer, int length) {
    PCPKT recPacket = (PCPKT) buffer;
    int pid = fork();
    switch(pid) {
    case 0:
        if (recPacket->type == RXMSG) {
            printf("TXMSG\n");
            txFile(socketFD, recPacket);
        } else if (recPacket->type == TXMSG) {
            printf("RXMSG\n");
            rxFile(socketFD, recPacket);
        } else if (recPacket->type == LIST) {
            printf("LIST\n");
            listFiles(socketFD);
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
