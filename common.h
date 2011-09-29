#ifndef COMMON_H
#define COMMON_H

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>

#define MAXBUFFSIZE     2048
#define COMMANDLENGTH   2
#define TRUE            1
#define FALSE           0
#define MAXBUFFSIZE     2048
#define TXMSG           0
#define RXMSG           1
#define LIST            2

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
typedef struct communicationPacket {
    int packetlength;
    char msgType;
    int totalPackets;
    char fileName[FILENAME_MAX];
} CPACKET, *PCPACKET;

/**
 * reads a file from filesystem and transmits it on the requested socket
 * @author Warren Voelkl
 */
void writeFileToSocket(FILE* pFile, int socketFD);

#endif
