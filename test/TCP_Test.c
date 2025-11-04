/******************************************************************
1] Install cJSON library
2] Compile with:
   gcc server.c -o server -lcjson -lpthread
******************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cjson/cJSON.h>
#include <sys/stat.h>

#define CSV_FILE "data_log.csv"

pthread_mutex_t file_lock = PTHREAD_MUTEX_INITIALIZER;

// Function to check if file exists
int file_exists(const char *filename)
{
	struct stat buffer;
	return (stat(filename, &buffer) == 0);
}

// Function to write data to CSV file
void log_to_csv(double T1, double P1, double T2, double P2, const char *timestamp)
{
	FILE *fp;
	pthread_mutex_lock(&file_lock); // prevent race conditions

	// If file exists, open in "w" mode to overwrite each run
	static int initialized = 0;
	if (!initialized)
	{
		fp = fopen(CSV_FILE, "w"); // overwrite if exists
		if (fp == NULL)
		{
			perror("Error opening file");
			pthread_mutex_unlock(&file_lock);
			return;
		}
		fprintf(fp, "Temperature1,Pressure1,Temperature2,Pressure2,Timestamp\n");
		initialized = 1;
	}
	else
	{
		fp = fopen(CSV_FILE, "a"); // append for next entries
		if (fp == NULL)
		{
			perror("Error opening file");
			pthread_mutex_unlock(&file_lock);
			return;
		}
	}

	fprintf(fp, "%lf,%lf,%lf,%lf,%s\n", T1, P1, T2, P2, timestamp);
	fclose(fp);
	pthread_mutex_unlock(&file_lock);
}

// Thread to handle each client
void *handle_client(void *arg)
{
	int connfd = *(int *)arg;
	free(arg);
	char buff[256];
	int n;
	cJSON *json = NULL;

	while (1)
	{
		bzero(buff, sizeof(buff));
		n = read(connfd, buff, sizeof(buff) - 1);
		if (n <= 0)
			break;

		printf("Received String: %s\n", buff);

		json = cJSON_Parse(buff);
		if (json == NULL)
		{
			printf("JSON Parsing Error: %s\n", cJSON_GetErrorPtr());
			break;
		}
		else
		{
			double T1 = cJSON_GetObjectItem(json, "T1")->valuedouble;
			double P1 = cJSON_GetObjectItem(json, "P1")->valuedouble;
			double T2 = cJSON_GetObjectItem(json, "T2")->valuedouble;
			double P2 = cJSON_GetObjectItem(json, "P2")->valuedouble;
			const char *timestamp = cJSON_GetObjectItem(json, "TStamp")->valuestring;

			printf("T1: %.2f\tP1: %.2f\tT2: %.2f\tP2: %.2f\tTime: %s\n", T1, P1, T2, P2, timestamp);

			// Log data into CSV file
			log_to_csv(T1, P1, T2, P2, timestamp);
		}

		strcpy(buff, "DONE\n");
		write(connfd, buff, strlen(buff));
		cJSON_Delete(json);
	}

	close(connfd);
	return NULL;
}

int main()
{
	int sockfd, *connfd;
	socklen_t len;
	struct sockaddr_in servaddr, cliaddr;
	pthread_t tid;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
	{
		perror("Socket Creation Failed");
		exit(EXIT_FAILURE);
	}

	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr("192.168.2.120");
	servaddr.sin_port = htons(5005);

	if (bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0)
	{
		perror("Bind Failed");
		close(sockfd);
		exit(EXIT_FAILURE);
	}

	if (listen(sockfd, 5) != 0)
	{
		perror("Listen Failed");
		close(sockfd);
		exit(EXIT_FAILURE);
	}

	printf("Server Listening on port 5005...\n");

	while (1)
	{
		len = sizeof(cliaddr);
		connfd = malloc(sizeof(int));
		*connfd = accept(sockfd, (struct sockaddr *)&cliaddr, &len);
		if (*connfd < 0)
		{
			perror("Accept Failed");
			free(connfd);
			continue;
		}

		printf("Client Connected: %s:%d\n", inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));

		if (pthread_create(&tid, NULL, handle_client, connfd) != 0)
		{
			perror("Thread Creation Failed");
			close(*connfd);
			free(connfd);
			continue;
		}

		pthread_detach(tid);
	}

	close(sockfd);
	return 0;
}

