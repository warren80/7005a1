#ifndef EPOLL_H
#define EPOLL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <errno.h>


#define TRUE        1
#define MAXEVENTS   64


/**
 * Any tasks that may want to be performed upon new client connecting.
 * currently empty.
 * @author Warren Voelkl
 */
void newConnectionTasks(int socketFD);
/**
 * Reads all data currently on socket and then sends it to the fnpointer
 * TODO limit to reading length of packet.
 * @author Warren Voelkl
 */
int readDataFromSocket(int socketFD, int (*fnPtr)(int, char*, int));
/**
 * Makes the socket Non blocking
 * @author Warren Voelkl
 */
void makeNonBlockingSocket (int socketDescriptor);
/**
 * Used to fill the addrinfo struct and validate success of call.
 * @author Warren Voelkl
 */
int getAddressResult(char *port, struct addrinfo **result);
/**
 * Binds socket and checks for success of operation
 * @author Warren Voelkl
 */
int createAndBind(char *port);
/**
 * Accepts all the incoming new sockets and sets them as nonblocking.
 * @author Warren Voelkl
 */
void processIncomingNewSocket(int socketFD, int epollFD);
/**
 * Main loop for this server.  Blocks until activity on a monitored socket
 * then it delets/adds and reads from sockets.  Upon a request to read from a socket
 * this function calls the functionPointer that was passed to it to process data.
 * @author Warren Voelkl
 */
void eventLoop(int socketFD, int epollFD, struct epoll_event *events, int (*pt2Func)(int, char*, int));
/**
 * Checks to see if socket was bound properly
 * @author Warren Voelkl
 */
int validateSocket(char* port);
/**
 * binds the server socket and sets it to be listened to.
 * @author Warren Voelkl
 */
void bindandListenSocket(int socketFD);
/**
 * sets the events to monitor on the epoll socket
 * @author Warren Voelkl
 */
void setEPollSocket(int epollFD, int socketFD, struct epoll_event **pevents);
/**
 * creates an epoll file descriptor
 * @author Warren Voelkl
 */
int createEPoll();

#endif // EPOLL_H
