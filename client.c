/*------------------------------------------------------------------------------
 * SOURCE FILE:    client.c
 *
 * PROGRAM:        client.out
 *
 * FUNCTIONS:      
 *
 * DATE:           Oct 2011
 *
 * REVISIONS:
 *
 * DESIGNERS:      Warren Voelkl & Matthew Hill
 *
 * PROGRAMMERS:    Warren Voelkl & Matthew Hill
 *
 * NOTES:          client for a tcp file transfer program.
 *
 *----------------------------------------------------------------------------*/
#include "client.h"

void usage(){
	fprintf(stderr,"clnt [hostname] [port]\n");
}

/*
 * Simply prints the menu.
 */
void printMenu(){
	printf("Enter a menu number:\n1 Download from server\n2 Upload to server\n3 Request file list\n0 Quit\n");
}

/*------------------------------------------------------------------------------
 * FUNCTION:      duplicate
 *
 * DATE:          Oct 2011
 *
 * REVISIONS:
 *
 * DESIGNERS:     Matthew Hill
 *
 * PROGRAMMERS:   Matthew Hill
 *
 * NOTES:         checks to see if a file already exists.
 *
 *----------------------------------------------------------------------------*/
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

/*------------------------------------------------------------------------------
 * FUNCTION:      requestFileList
 *
 * DATE:          Oct 2011
 *
 * REVISIONS:   
 *
 * DESIGNERS:     Matthew Hill
 *
 * PROGRAMMERS:   Matthew Hill
 *
 * NOTES:         Requests a file list from <sock> and prints it to screen.
 *
 *----------------------------------------------------------------------------*/
int requestFileList(int sock){
    char lst = LIST;
    PFTPKT incPacket = malloc(sizeof(FTPKT));
    PCPKT packet = malloc(sizeof(CPKT));
    int i = 0, count;
    /* packet is a request list */
    packet->type = lst;
    packet->pl = sizeof(char) + sizeof(int);
    count = write(sock, (void*)packet, packet->pl);
    if(count == -1){
        printf("Could not write to server.\n");
    }
    /* read the file list from socket */
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

/*------------------------------------------------------------------------------
 * FUNCTION:      receiveFile
 *
 * DATE:          Oct 2011
 *
 * REVISIONS:
 *
 * DESIGNERS:     Matthew Hill
 *
 * PROGRAMMERS:   Matthew Hill
 *
 * NOTES:         Reads from <sock> and writes the file to clientFiles/<name>.
 *
 *----------------------------------------------------------------------------*/
void receiveFile(int sock, char fileName[MAXBUFFSIZE]){
    int readCount, i = 0, totalPackets;
    PFTPKT incPacket = malloc(sizeof(FTPKT));
    FILE * pFile;
    float progressPercent;
    char filePath[MAXBUFFSIZE];
    /* deals with a newline we do not want on the end of our filename*/
    fileName[strlen(fileName)] = 0;
    /* concat the file name onto the path */
    snprintf(filePath, strlen(fileName)+13, "clientFiles/%s", fileName);
    
    pFile = fopen(filePath, "a+");
    if(pFile == NULL){
        printf("Failed to open/create file \"%s\".\n", filePath);
    }
    /* read the first packet and grab the totalPackets value for progress bar*/
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
void downloadFileList(int sock) {
    //char lst = LIST; //list file command
    //requestFileList(sock);
    //packet->type = lst;
    //packet->pl = sizeof(char);
}

/*------------------------------------------------------------------------------
 * FUNCTION:      getServerDataSocket
 *
 * DATE:          Oct 2011
 *
 * REVISIONS:
 *
 * DESIGNERS:     Warren Voelkl
 *
 * PROGRAMMERS:   Warren Voelkl
 *
 * NOTES:         Using the original socket, getServerDataSocket gets a work
 *                socket on port 7000 so we can transfer files.
 *
 *----------------------------------------------------------------------------*/
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

/*------------------------------------------------------------------------------
 * FUNCTION:      downloadFile
 *
 * DATE:          Oct 2011
 *
 * REVISIONS:  
 *
 * DESIGNERS:     Matthew Hill
 *
 * PROGRAMMERS:   Matthew Hill
 *
 * NOTES:         Gets and prepares a file name and path to communicate with
 *                the server and download a file.
 *
 *----------------------------------------------------------------------------*/
void downloadFile(int sock){
    char dl = RXMSG;//uploadCommand = TXMSG;
    char buffer[MAXBUFFSIZE];
    char filePath[MAXBUFFSIZE];
    PCPKT packet = malloc(sizeof(CPKT));
    int result;

    printf("Enter file name:\n");
    fflush(stdin);
    fgets(buffer, MAXBUFFSIZE - 1, stdin);
    /* trim a newline we do not want */
    buffer[strlen(buffer) - 1] = 0;
    snprintf(filePath, strlen(buffer)+13, "clientFiles/%s", buffer);
    packet->pl = strlen(buffer) + sizeof(unsigned int)*2;
    packet->type = dl;
    memcpy(packet->filename, buffer, strlen(buffer));
    result = write(sock, (void *) packet, sizeof packet);
    if (result != sizeof packet) {
        printf("failed to read all date from socket\n");
        return;
    }
    /* get the socket for doing work */
    sock = getServerDataSocket(sock);
    if (sock == -1) {
        printf("Failed to get new socket");
    }

    printf("Downloading %s from server...\n", buffer);
    /* pass the name of the file to the function that will write to file */
    receiveFile(sock, buffer);
    close(sock);
}

/*------------------------------------------------------------------------------
 * FUNCTION:      uploadFile
 *
 * DATE:
 *
 * REVISIONS:
 *
 * DESIGNERS:     X
 *
 * PROGRAMMERS:   X
 *
 * NOTES:         nothing in this function yet
 *
 *----------------------------------------------------------------------------*/
void uploadFile(int sock){

}



/*------------------------------------------------------------------------------
 * FUNCTION:      main
 *
 * DATE:          Oct 2011
 *
 * REVISIONS:
 *
 * DESIGNERS:     Matthew Hill
 *
 * PROGRAMMERS:   Matthew Hill
 *
 * NOTES:         Main binds the first socket and runs the menu loop
 *
 *----------------------------------------------------------------------------*/
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
