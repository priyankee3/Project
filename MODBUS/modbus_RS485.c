#include"header.h"

void main()
{
	int fd,n;
	short int sa = 172, q = 2;
	struct termios rs485;
	char buf[256], msg[8]; 
	
	msg[0] = 0x01;
	msg[1] = 0x03;
	msg[2] = (sa >> 8 ) & 0x00FF;
	msg[3] = sa & 0x00FF;
	msg[4] = ( q >> 8 ) & 0x00FF;
	msg[5] = q & 0x00FF;
	msg[6] = 0xC9;
	msg[7] = 0xC5;

	// Open Serial Port
	fd = open("/dev/ttyUSB0", O_RDWR);
	if( fd <= 0 ) {
		perror("Port Status:");
		return;
	}

	// Get current Setting 
	if( tcgetattr(fd, &rs485) != 0 ) {
		perror("tcgetattr status:");
		close(fd);
		return;
	}

	// Configure port
	cfsetispeed(&rs485, B9600);
	cfsetospeed(&rs485, B9600);

	rs485.c_cflag = (rs485.c_cflag & ~CSIZE) | CS8;	// 8 bit char
	rs485.c_iflag &= ~IGNBRK;	// Disable Break processing
	rs485.c_lflag = 0;	// nosignaling chars, no echo
	rs485.c_oflag = 0;	// no remapping, no delays
	rs485.c_cc[VMIN] = 0;	// non-blocking read
	rs485.c_cc[VTIME] = 20;	// 0.5 second read timeout 

	rs485.c_iflag &= ~(IXON | IXOFF | IXANY);	// no XON/XOFF
	rs485.c_cflag |= (CLOCAL | CREAD);	//ignore modem controls
	rs485.c_cflag &= ~(PARENB | PARODD);	//no parity
	rs485.c_cflag &= ~CSTOPB;	// 1 stop bit
	rs485.c_cflag &= ~CRTSCTS;	// no RTS/CTS

	// Applying attributes
	if( tcsetattr(fd, TCSANOW, &rs485) != 0 ) {
		perror("tcsetattr");
		close(fd);
		return;
	}

	// Write data
	int rc = write(fd, msg, strlen(msg));
	if( rc < 0 ) {
		perror("write status");
	}

	tcdrain(fd);	//wait until all data is transmitted

	// Then read response
	memset(buf, 0, sizeof(buf));
	n = read( fd, buf, sizeof(buf) );
	if( n > 0 ) {
		printf("Received: %s\n", buf);
	}
	else if( n < 0 ) {
		perror("read");
	}
	else 
		printf("No data received\n");
	
	close(fd);
}
