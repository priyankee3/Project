#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<fcntl.h>
#include<linux/serial.h>
#include<sys/ioctl.h>	// include defination for RS485 ioctls: TIOCGRS485 and TIOCSRS485
#include<termios.h>	// For Configuring USB
#include<string.h>
#include<stdint.h>

// Typedef 
typedef char s8;
typedef unsigned char u8;
typedef short int s16;
typedef unsigned short int u16;
typedef int s32;
typedef unsigned int u32;
typedef float f32;
typedef double f64;

// macros
#define RD_HLD_RG 0x03

// Function Prototype
uint16_t modbus_crc(uint8_t *, int);
void control_rts(int, int);
void modbusInit(int);
void modbus_req(s32 si, u16 sa, u16 qty);
void hex(int);

// Varibles 
int n;
struct termios rs485;
uint8_t msg[8];
uint8_t buf[256];

// User-defined Variables
struct hex_float {
	char value[0];
	float result;
}uni;
