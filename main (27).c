#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netdb.h>
#include <unistd.h>
#define MAXLINE 100
#define LISTENQ 10

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "usage: %s <domain name>\n", argv[0]);
        exit(0);
    }

    struct addrinfo *p, *listp, hints;
    char buf[MAXLINE];
    int rc, flags, connfd;
    int listenfd, optval = 1;
    socklen_t clientlen;

    /* Get a list of potential server addresses */
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_socktype = SOCK_STREAM;             /* Accept connections */
    hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG; /* ... on any IP address */
    hints.ai_flags |= AI_NUMERICSERV;            /* ... using port number */
    getaddrinfo(NULL, argv[1], &hints, &listp);

    /* Walk the list for one that we can bind to */
    for (p = listp; p; p = p->ai_next)
    {
        /* Create a socket descriptor */
        if ((listenfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0)
            continue; /* Socket failed, try the next */

        /* Bind the descriptor to the address */
        if (bind(listenfd, p->ai_addr, p->ai_addrlen) == 0)
            break;       /* Success */
        close(listenfd); /* Bind failed, try the next */
    }

    /* Clean up */
    freeaddrinfo(listp);
    if (!p) /* No address worked */
        return -1;

    /* Make it a listening socket ready to accept connection requests */
    if (listen(listenfd, LISTENQ) < 0)
    {
        close(listenfd);
        return -1;
    }

    while (1)
    {
        socklen_t sz = sizeof(p->ai_addr);
        connfd = accept(listenfd, p->ai_addr, &sz);
        send(connfd, "Hello From the Server\n", strlen("Hello From the Server\n"), 0);
        close(connfd);
    }
    exit(0);
}