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

#define TRUE 1
#define MAXEVENTS 64

void newConnectionTasks(int socketFD);
int readDataFromSocket(int socketFD, int (*fnPtr)(int, char*, int));
void makeNonBlockingSocket (int socketDescriptor);
int getAddressResult(char *port, struct addrinfo **result);
int createAndBind(char *port);
void processIncomingNewSocket(int socketFD, int epollFD);
void eventLoop(int socketFD, int epollFD, struct epoll_event *events, int (*pt2Func)(int, char*, int));
int validateSocket(char* port);
void bindandListenSocket(int socketFD);
void setEPollSocket(int epollFD, int socketFD, struct epoll_event **pevents);
int createEPoll();

#endif // EPOLL_H
