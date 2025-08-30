#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<mosquitto.h>

// typedef
typedef char s8;
typedef unsigned char u8;
typedef short int s16;
typedef unsigned short int u16;
typedef int s32;
typedef unsigned int u32;
typedef float f32;
typedef double d64;

// Functions used 
void on_connect(struct mosquitto *, void *, int);
void on_publish(struct mosquitto *, void *, int);

