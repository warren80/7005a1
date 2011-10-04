#include "common.h"

/*
typedef struct ftpacket {
    unsigned int pl;
    char data[FILENAME_MAX];
} FTPKT, *PFTPKT;
*/

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
    int i, totalPackets;
    char c;
    PFTPKT packet = malloc(sizeof(FTPKT));
    packet->packetNum  = totalPackets = fileSize(pFile)/DATABUFSIZE;
    packet->pl = DATABUFSIZE;
    while(packet->packetNum != 0){
        i = 0;
        for(; i != DATABUFSIZE; ++i){
            c = fgetc(pFile);
            if(c == EOF){
                break;
            }
            packet->data[i] = c;
        }
        printf("Write()[%i/%i;\n", packet->packetNum, totalPackets);
        write(socketFD, packet, MAXPACKETSIZE);
        packet->packetNum--;
    }
    i = 0;
    c = fgetc(pFile);
    while(c != EOF){
        packet->data[i++] = c;
        printf("%c", c);
        c = fgetc(pFile);
    }
    
    packet->pl = i;
    printf("lastpacketsize:%i", packet->pl);
    write(socketFD, packet, MAXPACKETSIZE);
    close(socketFD);

}

void printSocketData(int sd, struct sockaddr *in_addr) {
    socklen_t socketLength;
    int connectResult;
    char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
    socketLength = sizeof(in_addr);
    connectResult = getnameinfo (in_addr, socketLength,
                     hbuf, sizeof hbuf,
                     sbuf, sizeof sbuf,
                     NI_NUMERICHOST | NI_NUMERICSERV);
    if (connectResult == 0) {
        printf("connection on descriptor %d "
               "(host=%s, port=%s)\n", sd, hbuf, sbuf);
    } else {
        printf("u have no clue");
    }
}

int allowManyBinds(int socketFD) {
    int result;
    int optval = 1;
    result = setsockopt(socketFD, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);
    if(result == -1) {
        printf("sockop error\n");
        return -1;
    }
    return 0;
}
