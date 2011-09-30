#ifndef COMMON_H
#define COMMON_H

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>

#define MAXBUFFSIZE     2048
#define COMMANDLENGTH   2
#define TRUE            1
#define FALSE           0
#define MAXBUFFSIZE     2048
#define TXMSG           0
#define RXMSG           1
#define LIST            2
#define DATABUFSIZE     4096

/**
 * int packetlength will be used to indicate the length of the filename
 * char msgType used to indicate the type of messages that may be sent
 *   TXMSG for sending to server
 *   RXMSG for recieving from server
 *   LIST to list files in the files/ directory
 * int totalPackets is to indicate the total number of packets to send for the completed file.
 * char *fileName for the filename terminated with a null value
 * @author Warren Voelkl
 **/

#pragma pack(push)
#pragma pack(1)
typedef struct commpacket {
    unsigned int pl;
    char type;
    char filename[FILENAME_MAX];
} CPKT, *PCPKT;

typedef struct ftpacket {
    unsigned int packetLength;
    unsigned int packetNum;
    char data[DATABUFSIZE];
} FTPKT, *PFTPKT;
#pragma pack(pop)

/**
 * reads a file from filesystem and transmits it on the requested socket
 * @author Warren Voelkl
 */
void writeFileToSocket(FILE* pFile, int socketFD);
void printSocketData(int sd, struct sockaddr *in_addr);

#endif
