#include"header.h"

int main()
{

	// Variables used
	s32 sockfd,connfd,len;	// scokfd is  for file descriptor for Socket
	struct sockaddr_in servaddr, cliaddr;	// Structure for adding information about ip address, port no, protocol type
	s8 buff[128];

	// Creating socket for communication
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0 ){
		printf("Socket creation failed......\n");
		return 0;
	}

	// Assign IP Port
	servaddr.sin_family = AF_INET;	//IPv4
	servaddr.sin_port = htons(5000);	// assigning port number
	//servaddr.sin_addr.s_addr = inet_addr("192.168.1.110");	//Assiging IP address
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);	//Assiging IP address

	// Binding Socket with IP 
	if(bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) != 0){
		printf("Binding socket failed.....\n");
		return 0;
	}

	// Server is ready to listen to 5 connection
	if(listen(sockfd, 5)){
		printf("Failed to connection.....\n");
		return 0;
	}
	

	// Accept the data packet from client and verification
	len = sizeof(cliaddr);	
	connfd = accept(sockfd, (struct sockaddr*)&cliaddr, &len);
	if( connfd < 0 ){
		printf("Server acception failed.....\n");
		return 0;
	}

	read(connfd, buff, sizeof(buff));	// Reading data from device
	printf("In server data received %s\n", buff);	
	bzero(buff, sizeof(buff));
	strcpy(buff,"Done");
	write(connfd, buff, sizeof(buff));	// Writing data from device

	close(connfd);	// closing connection with device
	close(sockfd);	// closing socket for commuincation
}
