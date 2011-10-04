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

int getClientSocket(struct sockaddr_in addr_in) {
    int sd;
    socklen_t socketLength;
    int clientEndport, result;

    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Cannot create socket");
        exit(1);
    }
    socketLength = sizeof(addr_in);
    clientEndport = addr_in.sin_port;
    addr_in.sin_port = htons(CLIENTPORT);
    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Cannot create socket");
        exit(1);
    }
    result = bind(sd,(struct sockaddr*)&addr_in,socketLength);
    if (result == -1) {
        fprintf(stderr, "Can't bind to port 7000\n");
        perror("connect");
        exit(1);
    }
    addr_in.sin_port = clientEndport;

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
    FILE* pFile = popen("ls -l files", "r");
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
    struct sockaddr_in addr_in;
    PCPKT recPacket = (PCPKT) buffer;
    int pid, port;
    char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];

    socklen_t socketLength;
    socketLength = sizeof(addr_in);
    getpeername(socketFD, (struct sockaddr*)&addr_in, &socketLength);

    getnameinfo((const struct sockaddr *) &addr_in, sizeof(addr_in), hbuf, sizeof(hbuf), sbuf, sizeof(sbuf), NI_NUMERICHOST | NI_NUMERICSERV);
    port = atoi(sbuf);
    if (write(socketFD, &port, sizeof(int)) != sizeof(int)) {
        printf("error\n write port back;");
    }
    sleep(2);
    pid = fork();

    //not currently checking for boundries
    switch(pid) {
    case 0:
        if (buffer != NULL) {
            free(buffer);
        }
        return 0;
    case -1:
        perror("Fork");
        abort();
        return 1;
    default:
        close(socketFD);
        socketFD = getClientSocket(addr_in);
        if (recPacket->type == RXMSG) {
            printf("Recieve File\n");
            txFile(socketFD, recPacket);
        } else if (recPacket->type == TXMSG) {
            printf("Sending File\n");
            rxFile(socketFD, recPacket);
        } else if (recPacket->type == LIST) {
            printf("List Files\n");
            listFiles(socketFD);
        } else {
            printf("Invalid request\n");
        }
        //write(1, buffer, length);
        printf("\n");
        exit(EXIT_SUCCESS);
    }
    return 0;
}

int main(int argc, char *argv[]) {

    int (*fnPtr)(int, char*, int) = parseClientRequest;
    if (argc != 1) {
        fprintf (stderr, "Usage: %s\n", argv[0]);
        exit (EXIT_FAILURE);
    }

    startServer(SERVERPORT, fnPtr);
    return EXIT_SUCCESS;
}
