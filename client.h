#ifndef CLIENT_H
#define CLIENT_H

#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <errno.h>
#include "common.h"

int getServerDataSocket(int socketFD);
void downloadFileList(int sock);
void downloadFile(int sock);

#endif
