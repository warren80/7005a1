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
#define MAXPACKETSIZE   ((2 * sizeof(unsigned int)) + DATABUFSIZE)
#define SERVERPORT      7001
#define CLIENTPORT      7000

/**
 * TXMSG for sending to server
 * RXMSG for recieving from server
 * LIST to list files in the files/ directory
 * @author Warren Voelkl
 **/
#pragma pack(push)
#pragma pack(1)     //removes padding from structures for easy casting of char *s
typedef struct commpacket {
    unsigned int pl;
    char type;
    char filename[FILENAME_MAX];
} CPKT, *PCPKT;

typedef struct ftpacket {
    unsigned int pl;
    unsigned int packetNum;
    char data[DATABUFSIZE];
} FTPKT, *PFTPKT;
#pragma pack(pop)

/**
 * reads a file from filesystem and transmits it on the requested socket
 * @author Warren Voelkl
 */
void writeFileToSocket(FILE* pFile, int socketFD);
/**
 * Function used in error checking that retrieves port from sockaddr
 * @author Warren Voelkl
 */
void printSocketData(int sd, struct sockaddr *in_addr);
/**
 * sets the socket option to reuse
 * @author Warren Voelkl
 * @return -1 on failure 0 on success
 */
int allowManyBinds(int socketFD);
/**
 * gets the filesize from a filepointer.
 * @author Warren Voelkl
 * @return the size of a file
 */
int fileSize(FILE * pFile);

#endif
