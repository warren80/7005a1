#include "epoll.h"
#define MAXBUFFSIZE 2048

void newConnectionTasks(int socketFD) {
    /* if you want to tell the newly connected socket anything
    this is the spot */
}

void startServer(int port,int (*fnPtr)(int, char*, int)) {
    struct epoll_event *events;
    int socketFD;
    int epollFD;
    socketFD = validateSocket(port);
    bindandListenSocket(socketFD);
    epollFD = createEPoll();
    setEPollSocket(epollFD, socketFD, &events);
    eventLoop(socketFD, epollFD, events, fnPtr);
    close(socketFD);
}



int readDataFromSocket(int socketFD, int (*fnPtr)(int, char*, int)) {
    ssize_t length = 0;
    char *buf;
    buf = malloc(sizeof(char) * MAXBUFFSIZE);
    if (buf == NULL) {
        perror("MALLOC");
        abort();
    }


    while (1) {
        ssize_t count;
        count = read (socketFD, &buf[length], MAXBUFFSIZE - length);
        length += count;
        if (length == MAXBUFFSIZE) {
            printf("hope this never is reached");
            //some error condition here
        }
        switch (count) {
        case -1:
            if (errno != EAGAIN) {
                perror ("read");
                return 1;
            }
            if (length != 0) {
                return fnPtr(socketFD, buf, length);
            }
            //not sure what to do here so falling throught
        case 0:
            return 1;
        default:
            continue;
        }
    }
    return 0;
}

void makeNonBlockingSocket (int socketDescriptor) {
    int flags, result;
    flags = fcntl (socketDescriptor, F_GETFL, 0);
    if (flags == -1) {
        perror ("fcntl");
        abort ();
    }

    flags |= O_NONBLOCK;
    result = fcntl (socketDescriptor, F_SETFL, flags);
    if (result == -1) {
        perror ("fcntl");
        abort ();
    }
    return;
}

int getAddressResult(int port, struct addrinfo **result) {
    struct addrinfo hints;
    int returnValue;
    char sPort[6];
    memset (&hints, 0, sizeof (struct addrinfo));
    hints.ai_family = AF_UNSPEC;     /* Return IPv4 and IPv6 choices */
    hints.ai_socktype = SOCK_STREAM; /* We want a TCP socket */
    hints.ai_flags = AI_PASSIVE;     /* All interfaces */
    sprintf(sPort, "%d",port);
    returnValue = getaddrinfo (NULL, sPort, &hints, result);
    if (returnValue != 0) {
        fprintf (stderr, "getaddrinfo: %s\n", gai_strerror (returnValue));
        return -1;
    }
    return 0;
}

int createAndBind(int port) {

    struct addrinfo *result, *rp;
    int socketFD;

    if (getAddressResult(port, &result) == -1) { return -1; }

    for (rp = result; rp != NULL; rp = rp->ai_next) {
        socketFD = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (socketFD == -1) {
            continue;
        }
        if (bind (socketFD, rp->ai_addr, rp->ai_addrlen) == 0) {
            /* We managed to bind successfully! */
            break;
        }
        close (socketFD);
    }

    if (rp == NULL) {
        fprintf (stderr, "Could not bind\n");
        return -1;
    }
    freeaddrinfo (result);
    return socketFD;
}

void processIncomingNewSocket(int socketFD, int epollFD) {
    while (1) {
        struct epoll_event event;
        struct sockaddr in_addr;
        socklen_t in_len;
        int infd, epollSuccess, connectResult;
        char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
        in_len = sizeof(in_addr);
        infd = accept (socketFD, &in_addr, &in_len);
        if (infd == -1) {
            if ((errno == EAGAIN) ||
                    (errno == EWOULDBLOCK)) {
                /* We have processed all incoming
                   connections. */
                break;
            } else {
                perror ("accept");
                break;
            }
        }
        connectResult = getnameinfo (&in_addr, in_len,
                         hbuf, sizeof hbuf,
                         sbuf, sizeof sbuf,
                         NI_NUMERICHOST | NI_NUMERICSERV);
        if (connectResult == 0) {
            printf("Accepted connection on descriptor %d "
                   "(host=%s, port=%s)\n", infd, hbuf, sbuf);
            newConnectionTasks(infd);
        }
        makeNonBlockingSocket (infd);

        event.data.fd = infd;
        event.events = EPOLLIN | EPOLLET;
        epollSuccess = epoll_ctl (epollFD, EPOLL_CTL_ADD, infd, &event);
        if (epollSuccess == -1) {
            perror ("epoll_ctl");
            abort ();
        }
    }
}

void eventLoop(int socketFD, int epollFD, struct epoll_event *events, int (*fnPtr)(int, char*, int)) {
    while (TRUE) {
        int n, i;

        n = epoll_wait (epollFD, events, MAXEVENTS, -1);
        for (i = 0; i < n; i++) {
            if ((events[i].events & EPOLLERR) ||
                    (events[i].events & EPOLLHUP) ||
                    (!(events[i].events & EPOLLIN))) {
                /* An error has occured on this fd, or the socketFD is not
                   ready for reading (why were we notified then?) */
                fprintf (stderr, "epoll error\n");
                close (events[i].data.fd);
                continue;
            } else if (socketFD == events[i].data.fd) {
                processIncomingNewSocket(socketFD, epollFD);
                continue;
            } else {
                //Read all data from socket and do something with it
                if (readDataFromSocket(events[i].data.fd, fnPtr)) {
                    printf ("Closed connection on descriptor %d\n",
                            events[i].data.fd);
                    close (events[i].data.fd);
                }
            }
        }
    }
    free (events);
}

int validateSocket(int port) {
    int socketFD = createAndBind (port);
    if (socketFD == -1) {
        abort ();
    }
    return socketFD;
}

void bindandListenSocket(int socketFD) {
    int result;
    makeNonBlockingSocket (socketFD);

    result = listen (socketFD, SOMAXCONN);
    if (result == -1) {
        perror ("listen");
        abort ();
    }
}

int createEPoll() {
    int epollFD = epoll_create1 (0);
    if (epollFD == -1) {
        perror ("epoll_create");
        abort ();
    }
    return epollFD;
}

void setEPollSocket(int epollFD, int socketFD, struct epoll_event **pevents) {

    struct epoll_event event;
    int result;

    event.data.fd = socketFD;
    event.events = EPOLLIN | EPOLLET;
    result = epoll_ctl (epollFD, EPOLL_CTL_ADD, socketFD, &event);
    if (result == -1) {
        perror ("epoll_ctl");
        abort ();
    }

    /* Buffer where events are returned */
    *pevents = calloc (MAXEVENTS, sizeof event);
}
