#ifndef SERVER_H
#define SERVER_H

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "common.h"

#define CLIENTPORT 7000

/**
 *  Called when server is about to recieve a file.
 *  saves file to files/ folder
 *  @author Warren Voelkl
 */
void rxFile(int socketFD, PCPKT packet);
/**
 *  Called when server is requested to send a file
 *  Opens the requested file for reading.
 *  @author Warren Voelkl
 */
void txFile(int socketFD, PCPKT packet);
/**
 *  Called when server is requested to send a list of files (LS)
 *  calls the ls bash call opens a file pointer to the output then
 *  transmits contents back on socket.
 *  @author Warren Voelkl
 */
void listFiles(int socketFD);
/**
 * Connects a socket to client on port 7000
 * @author Warren Voelkl
 */
int getClientSocket(int socketFD);

int parseClientRequest(int socketFD, char *buffer, int length);
int main(int argc, char *argv[]);

#endif

