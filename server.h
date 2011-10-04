#ifndef SERVER_H
#define SERVER_H

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "common.h"

#define CLIENTPORT 7000
#define SERVERPORT 7001

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
 * retrieves the address info from the server socket then creats a new socket
 * and binds to port 7000 and the servers ip.  Then it connects to the client.
 * if any system calls fail the process ends.
 * @author Warren Voelkl
 * @return socket descriptor from port 7000 to client.
 */
int getClientSocket(struct sockaddr_in addr_in);

/**
 * Takes a packet from the server.  If multiple packets have been recieves it
 * just takes the first data and tosses the rest leave the client.
 * then it forks a new process and either returns to the epoll loop or starts a data transmission
 * return 0 on success exit(1) on child success or -1 on fork failure.
 */
int parseClientRequest(int socketFD, char *buffer, int length);
/**
 * Passes a function pointer of parse Client Request into the
 * epoll server start function.
 * @return never returns
 * @author Warren Voelkl
 */
int main(int argc, char *argv[]);

#endif

