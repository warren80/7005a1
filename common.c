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
    PFTPKT packet = malloc(sizeof(FTPKT));
    packet->pl = fileSize(pFile)/DATABUFSIZE;
    int i = 0;
    int result;
    while((packet->data[i++] = getc(pFile)) != EOF) {

        if (i == DATABUFSIZE + 1) {
            printf("you never get here\n");
            --packet->pl;
            if (write(socketFD, (void *)packet, sizeof(i -1 + sizeof(unsigned int))) == -1) {
                printf("fucked\n");
            }
            --packet->pl;
            i = 0;
        }
    }
    printf("you are here\n");
    printf("packet number: %d\n data to be written %d\n", packet->pl, i -1 + sizeof(unsigned int));
    //printf("%s", packet->data);

    pclose(pFile);
    /*
    if ((result = write(socketFD, (void *) packet, sizeof(i -1 + sizeof(unsigned int)))) == -1) {
        printf("fucked\n");
    }*/

    if ((result = write(socketFD, (void *)&packet->pl , 4)) == -1) {
        printf("fucked\n");
    }
    printf ("result: %d\n", result);
    printf("first packet %d", packet->pl);

    if ((result = write(socketFD, (void *)packet->data , i - 1)) == -1) {
        printf("fucked\n");
    }

    printf("result: %d\n", result);
    sleep(10);
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
