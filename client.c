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
    double progressPercent;

    pFile = fopen(fileName, "a+");
    if(pFile == NULL){
        printf("Failed to open/create file \"%s\".\n", fileName);
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
        system("clear");
        /* calculate and print download progress */
        progressPercent = (incPacket->packetNum/totalPackets)*100;
	printf("%s:%i%%",fileName, progressPercent);
    }
    /* read last packet */
    for(i = 0; i != incPacket->pl; ++i){
        fprintf(pFile, "%c", incPacket->data[i]);
    }
    fclose(pFile);
}

/**
 * Requests a list of files before prompting the user to enter a file. The file
 * will then be downloaded into files/.
 */
void downloadFile(int sock){
    char dl = RXMSG;//uploadCommand = TXMSG;
    char lst = LIST; //list file command
    char buffer[MAXBUFFSIZE];
    int sd, result, count;

    struct sockaddr_in addr_in;
    socklen_t socklen = sizeof(addr_in);
    PCPKT packet = malloc(sizeof(CPKT));
    
    system("clear");
    packet->type = lst;
    packet->pl = sizeof(char);

    printf("Enter file name:");
    // errant newline must be delt with
    fflush(stdin);
    fgets(buffer, sizeof(buffer), stdin);
    fgets(buffer, sizeof(buffer), stdin);
    
    /* check to see if you already have the file you requested, if so do not redownload
    if(){
        printf("You already have a file named %s. In the current directory.");
        return 0;
    }
    */
    packet->pl = strlen(buffer) + sizeof(unsigned int) + 1;
    packet->type = dl;
    memcpy(packet->filename, buffer, strlen(buffer) + 1);
    count = write(sock,(void *) packet, packet->pl);
    if(count == -1){
        printf("could not write to socket\n");
    }

    printf("Downloading %s from server...\n", buffer);
    getpeername(sock, (struct sockaddr*)&addr_in, &socklen);
    addr_in.sin_port=htons(7000);
    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        printf("Cannot create socket");
        exit(1);
    }
    result = bind(sd,(struct sockaddr*)&addr_in,socklen);
    
    result = listen(sd, 1);
    result = accept(sd, (struct sockaddr*) &addr_in, &socklen);
    close(sd);
    receiveFile(result, buffer);
    close(result);
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
        int sock, srvrPort, menuSelection, menuLoop = 1;//, byteCount;
	struct sockaddr_in srvrAddr;
	struct hostent *server;

	sock = socket(AF_INET, SOCK_STREAM, 0);

        if(argc != 2){
            usage();
        }

	if (sock == 0){
                printf("Failed to create socket.\n");
		return 1;
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
				menuLoop = 0;
			default:
				printf("Invalid menue selection, please try again.");
				continue;
		}
		printMenu();
	}	

	return 0;
}
