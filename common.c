#include "common.h"

FILE* openFile(char* filename, char * access) {
    char file[FILENAME_MAX];
    printf("filename>%s<", filename);
    snprintf(file, FILENAME_MAX  ,"files/%s", filename);
    printf("path/filename>%s<", file);
    FILE * pFile = fopen(file, access);
    if (pFile == NULL) {
        
        printf("Could not open file named \"%s\".\n", file);
        abort();
    }
    printf("sending\"%s\"", file);
    return pFile;
}

void txFile(int socketFD, PCPKT packet) {
    char fileAccess[2];
    fileAccess[0] = 'r';
    fileAccess[1] = '\0';
    FILE * pFile = openFile(packet->filename, fileAccess);
    printf("file \"%s\".\n", packet->filename);
    if(pFile == NULL){
        printf("Could not open file named \"%s\".\n", packet->filename);
    }
    writeFileToSocket(pFile, socketFD);
}

void receiveFile(int sock, char fileName[MAXBUFFSIZE]){
    int readCount, i = 0, totalPackets;
    PFTPKT incPacket = malloc(sizeof(FTPKT));
    FILE * pFile;
    float progressPercent;
    char filePath[MAXBUFFSIZE];
    
    fileName[strlen(fileName)] = 0;
    
    snprintf(filePath, strlen(fileName)+13, "clientFiles/%s", fileName);
    
    pFile = fopen(filePath, "a+");
    if(pFile == NULL){
        printf("Failed to open/create file \"%s\".\n", filePath);
    }

    readCount = read(sock, incPacket, MAXPACKETSIZE);
    if(readCount == -1){
        printf("Failed to read\n");
        exit(1);
    }
    totalPackets = incPacket->packetNum;
    /* write a full packet and write to file before reading next packet */
    while(incPacket->packetNum != 0){
        for(i = 0; i != incPacket->pl; ++i){
            fprintf(pFile, "%c", incPacket->data[i]);
        }
        i = 0;
        readCount = read(sock, incPacket, MAXPACKETSIZE);
        /* calculate and print download progress */
        progressPercent = totalPackets - incPacket->packetNum;
        progressPercent = (progressPercent/totalPackets)*100;
        printf(".");
    }
    /* read last packet */
    for(i = 0; i != incPacket->pl; ++i){
        fprintf(pFile, "%c", incPacket->data[i]);
    }
    printf("Done\n");
    fclose(pFile);
    
}


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
        write(socketFD, packet, MAXPACKETSIZE);
        packet->packetNum--;
    }
    i = 0;
    c = fgetc(pFile);
    while(c != EOF){
        packet->data[i++] = c;
        c = fgetc(pFile);
    }
    
    packet->pl = i;
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
