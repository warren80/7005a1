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
void downloadFile(int sock){
    char dl = RXMSG;//uploadCommand = TXMSG;
    char lst = LIST; //list file command
    char buffer[MAXBUFFSIZE];
    int sd, readCount;
    int result;

    struct sockaddr_in addr_in;
    socklen_t socklen = sizeof(addr_in);
    PCPKT packet = malloc(sizeof(CPKT));
    PFTPKT buf = malloc(sizeof(FTPKT));
    
    system("clear");
    packet->type = lst;
    packet->pl = sizeof(char);

    requestFileList(sock);

    printf("Enter file name:");
    fflush(stdin);
    fgets(buffer, sizeof(buffer), stdin);
    fgets(buffer, sizeof(buffer), stdin);
    packet->pl = strlen(buffer) + sizeof(unsigned int) + 1;
    packet->type = dl;
    memcpy(packet->filename, buffer, strlen(buffer) + 1);
    write(sock,(void *) packet, packet->pl);
    printf("Downloading %s from server...\n", buffer);
    getpeername(sock, (struct sockaddr*)&addr_in, &socklen);
    addr_in.sin_port=htons(7000);
    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Cannot create socket");
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
        int sock, srvrPort;//, byteCount;
	struct sockaddr_in srvrAddr;
	struct hostent *server;
        //char buffer[MAXBUFFSIZE];
	int menuSelection, menuLoop = 1;
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == 0){
                //error("Failed to create socket.\n");
	}
	srvrPort = atoi(argv[2]);	
	server = gethostbyname(argv[1]);
	if(server == NULL){
                //error("Failed to get host by name.");
		return 0;
	}
	srvrAddr.sin_family = AF_INET;
	
	bcopy((char *)server->h_addr, (char *)&srvrAddr.sin_addr.s_addr, server->h_length);
    	srvrAddr.sin_port = htons(srvrPort);
    	if (connect(sock,(struct sockaddr *) &srvrAddr,sizeof(srvrAddr)) < 0){
                //error("Server not found. Address and/or port number may be incorrect.\n");
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
