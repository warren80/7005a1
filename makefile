CC=gcc
CFLAGS=-Wall

server: server.o common.o epoll.o
	$(CC) -o s.out epoll.o common.o server.o
client: client.o common.o
	$(CC) -o c.out common.o client.o
clean:
	rm -rf *.o
	rm -rf *.out
server.o:
	$(CC) $(CFLAGS) -o server.o -c server.c
common.o:
	$(CC) $(CFLAGS) -o common.o -c common.c
epoll.o:
	$(CC) $(CFLAGS) -o epoll.o -c epoll.c
client.o:
	$(CC) $(CFLAGS) -o client.o -c client.c


