#include "client.h"

/**
 * Simply prints the clients usage to stderr.
 */
void usage(){
	fprintf(stderr,"clnt [hostname] [port]\n");
}

/**
 * Simply prints the menu.
 */
void printMenu(){
	printf("Enter a menu number:\n1 Download from server\n2 Upload to server\n0 Quit\n");
}

/**
 * Requests a list of files before prompting the user to enter a file. The file
 * will then be downloaded into files/.
 */
void downloadFile(int sock){
    char dl = RXMSG;//uploadCommand = TXMSG;
    char buffer[MAXBUFFSIZE];
    int sd;
    int result;
    struct sockaddr_in addr_in;
    socklen_t socklen = sizeof(addr_in);
    PCPKT packet = malloc(sizeof(CPKT));
    PFTPKT buf = malloc(sizeof(FTPKT));
    //int readCount;

    system("clear");

    //readCount =
    //read(sock,buffer,MAXBUFFSIZE);
    //printf("%s", buffer);
    printf("Enter file name:");
    fflush(stdin);
    fgets(buffer, sizeof(buffer), stdin);
    fgets(buffer, sizeof(buffer), stdin);
    packet->pl = strlen(buffer) + sizeof(unsigned int) + 1;
    packet->type = dl;
    memcpy(packet->filename, buffer, strlen(buffer) + 1);
    write(sock,(void *) packet, packet->pl);

    printf("Downloading %s from server...\n", buffer);
    /*request file and save it*/
    printf("now listening for new connection\n");
    getpeername(sock, (struct sockaddr*)&addr_in, &socklen);
    addr_in.sin_port=htons(7000);
    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Cannot create socket");
        exit(1);
    }
    result = bind(sd,(struct sockaddr*)&addr_in,socklen);

    result = listen(sd, 1);
    result = accept(sd, (struct sockaddr*) &addr_in, &socklen);
    printf("result: %d\n", result);
    close(sd);
    //while (1) {


    printf("bytes read %d\n", read(result,(void *)&buf->pl,4));
    printf("unsigned int size: %d\n", sizeof(unsigned int));
    printf("first packet %d\n", buf->pl);

    read(result, (void *)&buf->data, 57);
    write(1,(void *)&buf->data, 57);
    //}

    printf("errno: %d\n", errno);
    //sleep(10000);
    //sd = accept(sock, &addr_in, &socklen);
    printf("socket: %d\n", sd);

}

/**
 * After prompting the user for a file, the file is written to the passed port.
 */
void uploadFile(int sock){
	FILE *pFile;
	char buffer[MAXBUFFSIZE];
	char path[MAXBUFFSIZE] = "files/";
        char fileName[FILENAME_MAX];
	char *n;

	system("clear");

        while(1){
		system("ls files");
		printf("Enter file name or enter an empty line to return to menu:");
		memset(buffer, '\0', MAXBUFFSIZE);
		fflush(stdin); /*why the hell will this not flush?*/
		fgets(fileName, sizeof(fileName), stdin);
		fgets(fileName, sizeof(fileName), stdin);
	  	if ((n = strchr(fileName, '\n')) != NULL) *n = '\0';
		strcpy(buffer, path);
		strcat(buffer, fileName);
		if((pFile=fopen(buffer, "r"))==NULL) {
    			printf("Cannot open %s.\n", fileName);
			continue;
    		}
		printf("Uploading file to server\n");
		writeFileToSocket(pFile, sock);
		printf("%s uploaded to server. Press any key to return to menu...\n", fileName);
		break;
	}
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
		menuSelection = getchar();
		switch(menuSelection){
			case '1': /*download*/
				downloadFile(sock);
				break;
			case '2': /*upload*/
				uploadFile(sock);
				break;
			case '0': /*exit*/
				menuLoop = 0;
			default:
				continue;
		}
		printMenu();
	}	

	return 0;
}
