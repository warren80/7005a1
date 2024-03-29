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

void downloadFile(int sock, char * filename){
    char dl = RXMSG;//uploadCommand = TXMSG;
    char buffer[MAXBUFFSIZE];
    char filePath[MAXBUFFSIZE];
    PCPKT packet = malloc(sizeof(CPKT));

    buffer[strlen(buffer) - 1] = 0;
    snprintf(filePath, strlen(buffer)+6, "files/%s", buffer);
    printf("file path to check:[%s]\n", filePath);
    packet->pl = strlen(buffer) + sizeof(unsigned int)*2;
    packet->type = dl;
    memcpy(packet->filename, buffer, strlen(buffer));

    if (sock == -1) {
        printf("Failed to get new socket");
    }
    printf("Downloading %s from server...\n", buffer);

    receiveFile(sock, filename);

    close(sock);
}

void rxFile(int socketFD, PCPKT packet) {
    //int workSocket = getServerDataSocket(socketFD);
    downloadFile(socketFD, packet->filename);

}

int getClientSocket(struct sockaddr_in addr_in) {
    int sd;
    struct sockaddr_in temp;
    socklen_t socketLength;
    int clientEndport, result;

    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Cannot create socket");
        exit(1);
    }

    socketLength = sizeof(addr_in);
    clientEndport = addr_in.sin_port;
    addr_in.sin_port = htons(CLIENTPORT);

    if (allowManyBinds(sd) == -1) {
        printf("Sockopt error\n");
        exit(1);
    }

    temp.sin_addr = addr_in.sin_addr;
    addr_in.sin_addr.s_addr= htonl(INADDR_ANY);
    result = bind(sd,(struct sockaddr*)&addr_in,socketLength);
    if (result == -1) {
        fprintf(stderr, "Can't bind to port 7000\n");
        perror("connect");
        exit(1);
    }
    printf("Client connected: %s\n", inet_ntoa(temp.sin_addr));
    addr_in.sin_port = clientEndport;
    addr_in.sin_addr = temp.sin_addr;
    printf("Client Port: %d\n", htons(clientEndport));
    if (connect (sd, (struct sockaddr *)&addr_in, socketLength) == -1) {
        fprintf(stderr, "Can't connect to client\n");
        perror("connect");
        exit(1);
    }

    return sd;
}

void listFiles(int socketFD) {
    PFTPKT packet = malloc(sizeof(FTPKT));
    int c = 0, i = 0, byteCount;
    FILE* pFile = popen("ls -l files", "r");
    if (pFile == NULL) {
        exit(EXIT_FAILURE);
    }
    while(1){
        c = getc(pFile);
        if(c == EOF){ break; }
        packet->data[i++] = c;
    }
    packet->pl = i; // packet length is data + 2 unsigned ints.
    packet->packetNum = 0;

    byteCount = write(socketFD,(void *) packet, packet->pl + (2*sizeof(packet->pl)));
    if(byteCount == -1){
        printf("failed to write to socket\n");
    }
}

int parseClientRequest(int socketFD, char * buffer, int length) {
    struct sockaddr_in addr_in;
    PCPKT recPacket = (PCPKT) buffer;
    int pid, port;
    char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];

    printf("Parse Client Request\n");

    socklen_t socketLength;
    socketLength = sizeof(addr_in);
    getpeername(socketFD, (struct sockaddr*)&addr_in, &socketLength);

    getnameinfo((const struct sockaddr *) &addr_in, sizeof(addr_in), 
	hbuf, sizeof(hbuf), sbuf, sizeof(sbuf), NI_NUMERICHOST 
	| NI_NUMERICSERV);

    port = atoi(sbuf);
    if (write(socketFD, &port, sizeof(int)) != sizeof(int)) {
        printf("error\n write port back;");
    }
    sleep(0);
    sleep(0);	
    pid = fork();
    //not currently checking for boundries
    switch(pid) {
    case 0:
        if (buffer != NULL) {
            free(buffer);
        }
        break;
    case -1:
        perror("Fork");
        abort();
        return 1;
    default:
        close(socketFD);
        socketFD = getClientSocket(addr_in);
        if (recPacket->type == RXMSG) {
            printf("Recieve File\n");
            rxFile(socketFD, recPacket);
        } else if (recPacket->type == TXMSG) {
            printf("Sending File\n");
            txFile(socketFD, recPacket);
        } else if (recPacket->type == LIST) {
            printf("List Files\n");
            listFiles(socketFD);
        } else {
            printf("Invalid request\n");
        }
        close(socketFD);
        printf("Communication Successful");

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
