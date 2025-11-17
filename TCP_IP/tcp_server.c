/******************************************************************
1] Install cJSON library
2] Compile need -lcjson because to link cJSON library
3] Compile need -lpthread because to link pthread Library
******************************************************************/
#include"header.h"

void* handle_client(void *arg)
{
	s32 connfd = *(int *)arg;
	s32 n;
	free(arg);	// Free the allocated memory for client file descriptor
	s8 buff[256];
	cJSON *json = NULL;	// file descriptor for JSON
	
	printf("Client Connected: %s:%d\n", inet_ntoa(cliaddr.sin_addr),ntohs(cliaddr.sin_port));
	// Read and Write with client
	while(1)
	{
		bzero(buff, sizeof(buff));
		n = read(connfd, buff, sizeof(buff));
		
		if( n <= 0 )
		{
			printf("Client Disconnected !!\n");
			break;
		}

		printf("Received String: %s\n", buff);
		
		// Prase the JSON data
		json = cJSON_Parse(buff);
		if( json == NULL )
		{
			printf("JSON Prasing Status: %s", cJSON_GetErrorPtr());
			bzero(buff, sizeof(buff));
			continue;
		}
		else
		{
			printf("Temperature T1:%f\t",cJSON_GetObjectItem(json,"T1")->valuedouble);
			printf("Pressure P1:%f\t",cJSON_GetObjectItem(json,"P1")->valuedouble);
			printf("Temperature T2:%f\t",cJSON_GetObjectItem(json,"T2")->valuedouble);
			printf("Pressure P2:%f\t",cJSON_GetObjectItem(json,"P2")->valuedouble);
			printf("Date & Time:%s\n",cJSON_GetObjectItem(json,"TStamp")->valuestring);
		}

		bzero(buff, sizeof(buff));
		strcpy(buff,"DONE\n");
		write(connfd, buff, strlen(buff));
		cJSON_Delete(json);
	}
	close(connfd);
}

int main()
{

	// Variables used
	s32 sockfd,*connfd,len;	// scokfd is  for file descriptor for Socket
	pthread_t tid;

	// Creating socket for communication
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0)
	{
		perror("Socket Creation");
		exit(EXIT_FAILURE);
	}

	// Assign IP and PORT
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr("192.168.2.120");
	//servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(10051);

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

	printf("Server Listening on port 5005...\n");
	
	while(1)
	{
		// Accept one client 
		len = sizeof(cliaddr);
		connfd = malloc(sizeof(s32));
		*connfd = accept(sockfd, (struct sockaddr*)&cliaddr, &len);
		if(*connfd < 0)
		{
			perror("Accept failed");
			free(connfd);
			continue;
		}
		
		// Creating Thread for client
		if(pthread_create(&tid, NULL, handle_client, connfd) != 0 )
		{
			perror("Thread Creation Status:");
			close(*connfd);
			free(connfd);
		}
		
		pthread_detach(tid);	//no need to join
	}
	
	close(sockfd);
}
