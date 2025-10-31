#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<fcntl.h>
#include<linux/serial.h>
#include<sys/ioctl.h>	// include defination for RS485 ioctls: TIOCGRS485 and TIOCSRS485
#include<termios.h>	// For Configuring USB
#include<string.h>

// macros
#define RD_HLD_RG 0x03


