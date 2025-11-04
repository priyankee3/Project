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

// ---------- Function to safely get JSON double ----------
double get_json_double(cJSON *json, const char *key)
{
	cJSON *item = cJSON_GetObjectItem(json, key);
	if (item && cJSON_IsNumber(item))
		return item->valuedouble;
	else
		return 0.0; // default if missing
}

// ---------- Function to safely get JSON string ----------
const char *get_json_string(cJSON *json, const char *key)
{
	cJSON *item = cJSON_GetObjectItem(json, key);
	if (item && cJSON_IsString(item))
		return item->valuestring;
	else
		return "N/A";
}

// ---------- CSV Logging Function ----------
void log_to_csv(double T1, double P1, double T2, double P2, const char *timestamp)
{
	static int initialized = 0;
	FILE *fp;

	pthread_mutex_lock(&file_lock);

	if (!initialized)
	{
		fp = fopen(CSV_FILE, "w"); // Overwrite existing file only once
		if (fp == NULL)
		{
			perror("Error opening CSV file");
			pthread_mutex_unlock(&file_lock);
			return;
		}
		fprintf(fp, "Temperature1,Pressure1,Temperature2,Pressure2,Timestamp\n");
		initialized = 1;
	}
	else
	{
		fp = fopen(CSV_FILE, "a"); // Append mode
		if (fp == NULL)
		{
			perror("Error opening CSV file");
			pthread_mutex_unlock(&file_lock);
			return;
		}
	}

	fprintf(fp, "%.2lf,%.2lf,%.2lf,%.2lf,%s\n", T1, P1, T2, P2, timestamp);
	fclose(fp);
	pthread_mutex_unlock(&file_lock);
}

// ---------- Thread Function ----------
void *handle_client(void *arg)
{
	int connfd = *(int *)arg;
	free(arg);

	char buff[512];
	int n;
	cJSON *json = NULL;

	while (1)
	{
		bzero(buff, sizeof(buff));
		n = read(connfd, buff, sizeof(buff) - 1);
		if (n <= 0)
			break; // client disconnected

		printf("Received String: %s\n", buff);

		json = cJSON_Parse(buff);
		if (json == NULL)
		{
			printf("JSON Parsing Error: %s\n", cJSON_GetErrorPtr());
			continue;
		}

		double T1 = get_json_double(json, "T1");
		double P1 = get_json_double(json, "P1");
		double T2 = get_json_double(json, "T2");
		double P2 = get_json_double(json, "P2");
		const char *timestamp = get_json_string(json, "TStamp");

		printf("T1: %.2f\tP1: %.2f\tT2: %.2f\tP2: %.2f\tTime: %s\n", T1, P1, T2, P2, timestamp);

		log_to_csv(T1, P1, T2, P2, timestamp);

		strcpy(buff, "DONE\n");
		write(connfd, buff, strlen(buff));

		cJSON_Delete(json);
	}

	close(connfd);
	return NULL;
}

// ---------- Main Function ----------
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
		if (!connfd)
		{
			perror("Malloc failed");
			continue;
		}

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

