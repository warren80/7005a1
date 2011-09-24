#ifndef SERVER_H
#define SERVER_H

#define CLIENTPORT 7000

/**
 * int packetlength will be used to indicate the length of the filename
 * char msgType used to indicate the type of messages that may be sent
 *   TXMSG for sending to server
 *   RXMSG for recieving from server
 *   LIST to list files in the files/ directory
 * int totalPackets is to indicate the total number of packets to send for the completed file.
 * char *fileName for the filename terminated with a null value
 * @author Warren Voelkl
 **/
typedef struct communicationPacket {
    int packetlength;
    char msgType;
    int totalPackets;
    char fileName[FILENAME_MAX];
} CPACKET, *PCPACKET;
/**
 * reads a file from filesystem and transmits it on the requested socket
 * @author Warren Voelkl
 */
void writeFileToSocket(FILE* pFile, int socketFD);
/**
 *  Called when server is about to recieve a file.
 *  saves file to files/ folder
 *  @author Warren Voelkl
 */
void rxFile(int socketFD, char *buffer, int length);
/**
 *  Called when server is requested to send a file
 *  Opens the requested file for reading.
 *  @author Warren Voelkl
 */
void txFile(int socketFD, char *buffer, int length);
/**
 *  Called when server is requested to send a list of files (LS)
 *  calls the ls bash call opens a file pointer to the output then
 *  transmits contents back on socket.
 *  @author Warren Voelkl
 */
void listFiles(int socketFD, char *buffer, int length);
/**
 * Connects a socket to client on port 7000
 * @author Warren Voelkl
 */
int getClientSocket(int socketFD);

int parseClientRequest(int socketFD, char *buffer, int length);
int main(int argc, char *argv[]);

#endif

