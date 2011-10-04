#include "client.h"

/*
 * Simply prints the clients usage to stderr.
 */
void usage(){
	fprintf(stderr,"clnt [hostname] [port]\n");
}

/*
 * Simply prints the menu.
 */
void printMenu(){
	printf("Enter a menu number:\n1 Download from server\n2 Upload to server\n0 Quit\n");
}

/*
 * Sends a file list requuest to the server at the passed socket and prints the 
 * returned results.
 */
void requestFileList(int sock){
    char lst = LIST;
    PFTPKT incPacket = malloc(sizeof(FTPKT));
    PCPKT packet = malloc(sizeof(CPKT));
    int i = 0;

    packet->type = lst;
    packet->pl = sizeof(char) + sizeof(int);
    write(sock, (void*)packet, packet->pl);
    read(sock, incPacket, MAXBUFFSIZE);
    printf("p:%i Available files:\n", incPacket->pl);
    for(i = 0; i < incPacket->pl; ++i){
        printf("%c", incPacket->data[i]);
    }
}

void receiveFile(int sock, char fileName[MAXBUFFSIZE]){
    int readCount, i = 0, totalPackets;
    PFTPKT incPacket = malloc(sizeof(FTPKT));
    FILE * pFile;
    pFile = fopen(fileName, "a+");
    printf("Read()first;\n");
    readCount = read(sock, incPacket, MAXPACKETSIZE);
    while(incPacket->packetNum != 0){
        printf("preloop:");
        for(i = 0; i != incPacket->pl; ++i){
            fprintf(pFile, "%c", incPacket->data[i]);
        }
        printf("postloop;");
        i = 0;
        printf("Read()%i;\n", incPacket->packetNum);
        readCount = read(sock, incPacket, MAXPACKETSIZE);
        printf("afterRead");
    }
    for(i = 0; i != incPacket->pl; ++i){
        fprintf(pFile, "%c", incPacket->data[i]);
    }
    fclose(pFile);
}

/**
 * Requests a list of files before prompting the user to enter a file. The file
 * will then be downloaded into files/.
 */


void downloadFileList(int sock) {
    //char lst = LIST; //list file command
    //requestFileList(sock);
    //packet->type = lst;
    //packet->pl = sizeof(char);
}

int getServerDataSocket(int socketFD) {
    int result, sd;
    int port;
    int count = 0;

    struct sockaddr_in srvaddr, oldAddr;
    //srvaddr.sin_addr.in_addr = INADDR_ANY;

    socklen_t socklen = sizeof(oldAddr);
    //getpeername(socketFD, (struct sockaddr *) &oldAddr, &socklen);

    while(1) {
        printf("moo\n");
        count += read(socketFD,(void *) &port + count, sizeof(int) - count);
        if (count == sizeof(int)) {
            printf("test\n");
            break;
        }
        printf("moo\n");
    }

    close(socketFD);

    bzero((char *)&srvaddr, sizeof(struct sockaddr_in));
    srvaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    srvaddr.sin_port = htons(32145);
    srvaddr.sin_family = AF_INET;
    printf("Port %d\n",port);

    printf("moo\n");
    printf ("reading port num %d\n", port);

/*
    int optval;
    int bah;
    optval = 1;
    bah = setsockopt(socketFD, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);
    if(bah == -1) {
        printf("sockop error\n");
    }

*/
    close(socketFD);



    //printf ("port x, %d", addr_in.sin_port=htons(7000));

    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Cannot create socket");
        exit(1);
    }
    printf("moo\n");

    result = bind(sd,(struct sockaddr*)&srvaddr,sizeof srvaddr);
    if (result == -1) {
        printf("failed to bind\n");
        return -1;
    }

    result = listen(sd, 1);
    if (result == -1) {
        printf("failed to listen\n");
        return -1;
    }
    result = accept(sd, (struct sockaddr*) &srvaddr, &socklen);
    if (result == -1) {
        printf("failed to accept\n");
        return -1;
    }
    printf("result: %d\nSocket: %d\n", result, sd);


    return result;

}

void downloadFile(int sock){
    char dl = RXMSG;//uploadCommand = TXMSG;
    char buffer[MAXBUFFSIZE];

    PCPKT packet = malloc(sizeof(CPKT));
    //PFTPKT buf = malloc(sizeof(FTPKT));
    
    //system("clear");
    printf("Enter file name:");
    fflush(stdin);
    fgets(buffer, sizeof(buffer), stdin);
    fgets(buffer, sizeof(buffer), stdin);
    packet->pl = strlen(buffer) + sizeof(unsigned int) + 1;
    packet->type = dl;
    memcpy(packet->filename, buffer, strlen(buffer) + 1);
    write(sock,(void *) packet, packet->pl);

    printf("wtf\n");
    sock = getServerDataSocket(sock);
    printf("Downloading %s from server...\n", buffer);
    if (sock == -1) {
        printf("Failed to get new socket");
    }
    receiveFile(sock, buffer);
    close(sock);
}

/**
 * After prompting the user for a file, the file is written to the passed port.
 */
void uploadFile(int sock){

}



/*
 * Opens a socket to the file server and prompts the user to upload or download.
 */
int main(int argc, char *argv[]){
        int sock, srvrPort;//, byteCount;
	struct sockaddr_in srvrAddr;
	struct hostent *server;
        //char buffer[MAXBUFFSIZE];
	int menuSelection, menuLoop = 1;
	sock = socket(AF_INET, SOCK_STREAM, 0);



        int optval;
        int bah;
        optval = 1;
        bah = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);
        if(bah == -1) {
            printf("sockop error\n");
        }

//        close(socketFD);


	if (sock == 0){
                printf("Failed to create socket.\n");
		return 1;
	}
	srvrPort = atoi(argv[2]);	
	server = gethostbyname(argv[1]);
	if(server == NULL){
                printf("Failed to get host by name.");
		return 1;
	}
	srvrAddr.sin_family = AF_INET;
	
	bcopy((char *)server->h_addr, (char *)&srvrAddr.sin_addr.s_addr, server->h_length);
    	srvrAddr.sin_port = htons(srvrPort);
    	if (connect(sock,(struct sockaddr *) &srvrAddr,sizeof(srvrAddr)) < 0){
                printf("Server not found. Address and/or port number may be incorrect.\n");
                return 1;
	}


	printMenu();
	while(menuLoop){
                scanf("%i", &menuSelection);
		switch(menuSelection){
			case 1: /*download*/
				downloadFile(sock);
				break;
			case 2: /*upload*/
				uploadFile(sock);
				break;
			case 0: /*exit*/
				menuLoop = 0;
			default:
				printf("-%i-", menuSelection);
				continue;
		}
		printMenu();
	}	

	return 0;
}
