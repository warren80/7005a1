#include "common.h"

void writeFileToSocket(FILE* pFile, int socketFD) {
    char buf[MAXBUFFSIZE];
    int i = 0;
    while((buf[i++] = getc(pFile)) != EOF) {
        if (i == MAXBUFFSIZE + 1) {
            write(socketFD, buf, MAXBUFFSIZE);
            i = 0;
        }
    }
    pclose(pFile);
    write(socketFD, buf, i-1);
}
