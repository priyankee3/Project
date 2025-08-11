#include"header.h"

int main()
{

	// Variables used
	s32 sockfd,connfd,len;	// scokfd is  for file descriptor for Socket
	struct sockaddr_in servaddr, cliaddr;	// Structure for adding information about ip address, port no, protocol type
	s8 buff[128];
	s8 *e = NULL;

	// Creating socket for communication
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0)
	{
		perror("Socket Creation");
		exit(EXIT_FAILURE);
	}

	// Assign IP and PORT
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(5000);

	// Bind Socket
	if(bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) != 0)
	{
		perror("Bind status");
		close(sockfd);
		exit(EXIT_FAILURE);
	}

	// Listen
	if(listen(sockfd, 5) != 0)
	{
		perror("Listen status");
		close(sockfd);
		exit(EXIT_FAILURE);
	}

	printf("Server Listening on port 5000...\n");

	// Accept one client 
	len = sizeof(cliaddr);
	connfd = accept(sockfd, (struct sockaddr*)&cliaddr, &len);
	if(connfd < 0)
	{
		perror("Accept failed");
		close(sockfd);
		exit(EXIT_FAILURE);
	}

	printf("Client Connected: %s:%d\n", inet_ntoa(cliaddr.sin_addr),ntohs(cliaddr.sin_port));
	
	// Read and Write with client
	s32 n;
	while(1)
	{
		n = read(connfd, buff, sizeof(buff)-1);

		printf("Received String: %s\n", buff);

		strcpy(buff,"DONE");
		write(connfd, buff, sizeof(buff));
	}

	close(connfd);
	close(sockfd);
}
