// All the required header files
#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/socket.h>	// For Creating socket
#include<arpa/inet.h>	// For converting IPs, Port no to network order
#include<string.h>
#include<cjson/cJSON.h>	// For Transmitting and receving data in JSON format
#include<pthread.h>	// for thread
//#include<netinet/in.h>

// Typedef
typedef unsigned char u8;
typedef char s8;
typedef unsigned short int u16;
typedef short int s16;
typedef unsigned int u32;
typedef int s32;
typedef float f32;
typedef double d64;

