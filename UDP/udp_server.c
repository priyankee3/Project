#include"header.h"

void main()
{
	struct sockaddr_in servaddr, cliaddr;
	s32 sockfd, len, n;
	s8 buffer[256];

	// Create Socket
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);	// Socket Created for IPv4 , UDP
	if(sockfd < 0)
	{
		perror("Socket Status:");
		exit(EXIT_FAILURE);
	}

	// Binding server with information
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = INADDR_ANY;	// For any IP of system
	servaddr.sin_port = htons(10051);

	if(bind( sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
	{
		perror("Bind failed");
		exit(EXIT_FAILURE);
	}

	len = sizeof(cliaddr);

	printf("UDP server listening on port 10051.....\n");

	while(1)
	{
		n = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&cliaddr, &len);
		buffer[n] = '\0';

		printf("Client: %s\n", buffer);

		sendto(sockfd, "Message received", 16, 0, (const struct sockaddr*)&cliaddr, len);
	}

	close(sockfd);

}
