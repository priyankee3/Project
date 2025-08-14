#include"header.h"

void main()
{
	struct sockaddr_in servaddr, cliaddr;
	s32 sockfd, len, n;
	s8 buffer[256];
	cJSON *json = NULL;	//file descriptor for JSON

	// Create Socket
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);	// Socket Created for IPv4 , UDP
	if(sockfd < 0)
	{
		perror("Socket Status:");
		exit(EXIT_FAILURE);
	}

	// Binding server with information
	servaddr.sin_family = AF_INET;
	//servaddr.sin_addr.s_addr = INADDR_ANY;	// For any IP of system
	servaddr.sin_addr.s_addr = inet_addr("192.168.1.120");	// For any IP of system
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
		// Paresing the JSON data
		json = cJSON_Parse(buffer);
		if( json == NULL )
		{
			printf("JSON Prasing Status: %s", cJSON_GetErrorPtr());
			cJSON_Delete(json);
		}
		else
		{
			printf("Temperature T1:%f\t",cJSON_GetObjectItem(json,"T1")->valuedouble);
			printf("Pressure P1:%f\t",cJSON_GetObjectItem(json,"P1")->valuedouble);
			printf("Temperature T2:%f\t",cJSON_GetObjectItem(json,"T2")->valuedouble);
			printf("Pressure P2:%f\t",cJSON_GetObjectItem(json,"P2")->valuedouble);
			printf("Date & Time:%s\n",cJSON_GetObjectItem(json,"TStamp")->valuestring);
		}
		

		sendto(sockfd, "Message received", 16, 0, (const struct sockaddr*)&cliaddr, len);
	}

	close(sockfd);
}
