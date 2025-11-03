#include "header.h"

int main()
{
	int fd;

	// ----------- Open Serial Port -----------
	fd = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY | O_SYNC);
	if (fd < 0) {
		perror("Port open failed");
		return -1;
	}

	// ---------- Initialize Modbus ----------
	modbusInit(fd);

	// ---------- Frequency ----------
	control_rts(fd, 1);	// Enable for transmission
	
	modbus_req(1, 172, 2);
	n = write(fd, msg, sizeof(msg));
	if( n < 0 ) {
		perror("Write:");
		close(fd);
		return 0;
	}
	
	tcdrain(fd);	// Wait until all data is Transmitted
	
	// Read Response
	memset(buf, 0, sizeof(buf));
	n = read(fd, buf, sizeof(buf));

	if ( n > 0 ) {
		printf("Received %X\n", buf);
		hex(n);
	}
	else if( n < 0 )
		perror("Read");
	else
		printf("Read Timeout\n");

	close(fd);
	return 0;
}

// ---------- MODBUS Initialization ----------
void modbusInit(int fd)
{
	// ----------- Configure UART 9600 8N1 -----------
	if (tcgetattr(fd, &rs485) != 0) {
		perror("tcgetattr failed");
		close(fd);
	}

	cfsetispeed(&rs485, B9600);
	cfsetospeed(&rs485, B9600);

	rs485.c_cflag = (rs485.c_cflag & ~CSIZE) | CS8; // 8 data bits
	rs485.c_iflag &= ~IGNBRK;
	rs485.c_lflag = 0;
	rs485.c_oflag = 0;
	rs485.c_cc[VMIN] = 0;
	rs485.c_cc[VTIME] = 20;  // 2 seconds timeout (0.1s * 20)

	rs485.c_iflag &= ~(IXON | IXOFF | IXANY);
	rs485.c_cflag |= (CLOCAL | CREAD);
	rs485.c_cflag &= ~(PARENB | PARODD); // No parity
	rs485.c_cflag &= ~CSTOPB;            // 1 stop bit
	rs485.c_cflag &= ~CRTSCTS;           // Disable HW flow control

	tcsetattr(fd, TCSANOW, &rs485);
}

// ---------- MODBUS Request Frame Setup ----------
void modbus_req(s32 si, u16 sa, u16 qty)
{
	u16 crc;
	// ----------- Build Modbus RTU Request -----------
	msg[0] = (char)si;                    // Slave ID
	msg[1] = 0x03;                    // Function Code (Read Holding Registers)
	msg[2] = (sa >> 8) & 0xFF;
	msg[3] = sa & 0xFF;
	msg[4] = (qty >> 8) & 0xFF;
	msg[5] = qty & 0xFF;
	// Compute CRC dynamically
	crc = modbus_crc(msg, 6);
	msg[6] = crc & 0xFF;              // CRC Low byte
	msg[7] = (crc >> 8) & 0xFF;       // CRC High byte

}

// ---------- CRC Calculation Function ----------
uint16_t modbus_crc(uint8_t *buf, int len)
{
	uint16_t crc = 0xFFFF;
	for (int pos = 0; pos < len; pos++) {
		crc ^= (uint16_t)buf[pos];
		for (int i = 0; i < 8; i++) {
			if (crc & 1)
				crc = (crc >> 1) ^ 0xA001;
			else
				crc >>= 1;
		}
	}
	return crc;
}

// ---------- Hex Display ----------
void hex(int len)
{
	for(int i=0; i < len ; i++)
		printf(" %02X",buf[i]);
	printf("\n");
}

// ---------- Control RTS Pins ----------
void control_rts(int fd, int enable)
{
	int flags;

	ioctl(fd, TIOCMGET, &flags);

	if(enable)
		flags |= TIOCM_RTS;	// Set RTS high (Enable Tx)
	else
		flags &= ~TIOCM_RTS;	// Set RTS low (Enable Rx)

	ioctl(fd, TIOCMSET, &flags);
}
