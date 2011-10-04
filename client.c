/*------------------------------------------------------------------------------
 * SOURCE FILE:    client.c
 *
 * PROGRAM:        client.out
 *
 * FUNCTIONS:      
 *
 * DATE:
 *
 * REVISIONS:
 *
 * DESIGNERS:
 *
 * PROGRAMMERS:
 *
 * NOTES:
 *
 *----------------------------------------------------------------------------*/
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
	printf("Enter a menu number:\n1 Download from server\n2 Upload to server\n3 Request file list\n0 Quit\n");
}

/*
 * duplicate returns 1 if the file already exists and 0 otherwise.
 */
int duplicate(char * filename){
    FILE* pFile = fopen(filename, "r");
    if (pFile == NULL){        
        fclose(pFile);        
        return 0;   
    }   
     return 1;
}

/*
 * Sends a file list requuest to the server at the passed socket and prints the 
 * returned results. Returns '1' if the server fails to list files.
 */
int requestFileList(int sock){
    char lst = LIST;
    PFTPKT incPacket = malloc(sizeof(FTPKT));
    PCPKT packet = malloc(sizeof(CPKT));
    int i = 0, count;

    packet->type = lst;
    packet->pl = sizeof(char) + sizeof(int);
    count = write(sock, (void*)packet, packet->pl);
    if(count == -1){
        printf("Could not write to server.\n");
    }

    count = read(sock, incPacket, MAXBUFFSIZE);
    if(count == -1){
        printf("Could not read from server.\n");
    }

    /* check to see that the server has the file
    if(server lists no files){
        printf("server has no files to download.\n");
        return 1;
    }
    */
    /* print the list of files sent from the server */
    printf("Available files:\n");
    for(i = 0; i < incPacket->pl; ++i){
        printf("%c", incPacket->data[i]);
    }
    return 0;
}

/*
 * Listens for packets on the passed socket and writes them to a file that is 
 * passed by name.
 */
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
    int port = 0;
    int count = 0;

    struct sockaddr_in srvaddr, oldAddr;
    //srvaddr.sin_addr.in_addr = INADDR_ANY;

    socklen_t socklen = sizeof(oldAddr);
    getpeername(socketFD, (struct sockaddr *) &oldAddr, &socklen);

    while(1) {
        count += read(socketFD,(void *) &port + count, sizeof(int) - count);
        if (count == sizeof(int)) {
            break;
        }
    }

    oldAddr.sin_port = htons(port);

    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Cannot create socket");
        exit(1);
    }
    if (allowManyBinds(sd) == -1) {
        printf("Sockopt error\n");
        return -1;
    }
    oldAddr.sin_addr.s_addr= htonl(INADDR_ANY);
    result = bind(sd,(struct sockaddr*)&oldAddr,sizeof oldAddr);
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
    close(socketFD);

    return result;

}

void downloadFile(int sock){
    char dl = RXMSG;//uploadCommand = TXMSG;
    char buffer[MAXBUFFSIZE];


    PCPKT packet = malloc(sizeof(CPKT));
    int result;
    printf("Enter file name:\n");
    // errant newline must be delt with
    fflush(stdin);
    fgets(buffer, MAXBUFFSIZE - 1, stdin);


    buffer[strlen(buffer) - 1] = 0;
    packet->pl = strlen(buffer) + sizeof(unsigned int)*2;
    packet->type = dl;
    memcpy(packet->filename, buffer, strlen(buffer));
    result = write(sock, (void *) packet, sizeof packet);
    if (result != sizeof packet) {
        printf("failed to read all date from socket\n");
        return;
    }

    sock = getServerDataSocket(sock);
    if (sock == -1) {
        printf("Failed to get new socket");
    }

    printf("Downloading %s from server...\n", buffer);

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
        int sock, srvrPort, menuSelection, menuLoop = 1;
	struct sockaddr_in srvrAddr;
	struct hostent *server;

        if(argc != 2){
            usage();
            return 1;
        }

        sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == 0){
                printf("Failed to create socket.\n");
		return 1;
	}

        if (allowManyBinds(sock) == -1) {
            printf("Failed to set sock opt\n");
            return 0;
        }

	srvrPort = SERVERPORT;
	server = gethostbyname(argv[1]);
	if(server == NULL){
                printf("Failed to get host by name.");
		return 1;
	}
	srvrAddr.sin_family = AF_INET;
	
	bcopy((char *)server->h_addr, (char *)&srvrAddr.sin_addr.s_addr, server->h_length);
    	srvrAddr.sin_port = htons(srvrPort);
    	if (connect(sock,(struct sockaddr *) &srvrAddr,sizeof(srvrAddr)) < 0){
                printf("Server not found. Address may be incorrect.\n");
                return 1;
	}

	printMenu();
	while(menuLoop){
                scanf("%i", &menuSelection);
                getchar();
		switch(menuSelection){
			case 1: /*download*/
				downloadFile(sock);
				break;
			case 2: /*upload*/
				uploadFile(sock);
				break;
                        case 3:
                                /* request server list */
                                printf("print the files that the server can send");
                                break;
			case 0: /*exit*/
				return 0;
			default:
				printf("Invalid menue selection, please try again.");
				continue;
		}
		printMenu();
	}	

	return 0;
}
