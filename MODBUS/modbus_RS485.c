#include "header.h"

int main()
{

	// ----------- Open Serial Port -----------
	fd = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY | O_SYNC);
	if (fd < 0) {
		perror("Port open failed");
		return -1;
	}

	// ---------- Initialize Modbus ----------
	modbusInit(fd);
	tcflush( fd, TCIFLUSH );
	tcsetattr(fd, TCSANOW, &rs485);

	while(1)
	{
		// ---------- Frequency ----------
		control_rts(fd, 1);	// Enable for transmission

		modbus_req(1, 172, 2);
		n = write(fd, msg, sizeof(msg));
		if( n < 0 ) {
			perror("Write:");
			close(fd);
			return 0;
		}


		t = time(NULL);	// Making it zero

		// Function to Set fd to read from device
		read_RS485();

		usleep(50000);	// Delay of more than char 3.5 for receving data
		if( n > 0 ) {
			// Read Response
			memset(buf, 0, sizeof(buf));
			n = read(fd, buf, sizeof(buf));

			if ( n > 0 ) {
				printf("Received size %d\n", n);
				hex(n);
				
				if(crc_check( buf, n))
					printf("Okay\n");
				else
					printf("Not okay\n");
			}
			else if( n < 0 )
				perror("Read");
			else
				printf("Read Timeout\n");

		}

		// ---------- Voltage ----------
		control_rts(fd, 1);	// Enable Transmission

		modbus_req( 1, 100, 6);
		n = write(fd, msg, sizeof(msg));
		if( n < 0 ) {
			perror("Write:");
			close(fd);
			return 0;
		}
		
		// Function to Set fd to read from device
		read_RS485();

		usleep(50000);	// Delay of more than char 3.5 for receving data
		if( n > 0 ) {
			// Read Response
			memset(buf, 0, sizeof(buf));
			n = read(fd, buf, sizeof(buf));

			if ( n > 0 ) {
				printf("Received size %d\n", n);
				hex(n);
				
				if(crc_check( buf, n))
					printf("Okay\n");
				else
					printf("Not okay\n");
			}
			else if( n < 0 )
				perror("Read");
			else
				printf("Read Timeout\n");

		}
		// ---------- Power Factor ----------
		control_rts(fd, 1);	// Enable Transmission

		modbus_req( 1, 134, 6);
		n = write(fd, msg, sizeof(msg));
		if( n < 0 ) {
			perror("Write:");
			close(fd);
			return 0;
		}
		
		// Function to Set fd to read from device
		read_RS485();
	
		usleep(50000);	// Delay of more than char 3.5 for receving data
		if( n > 0 ) {
			// Read Response
			memset(buf, 0, sizeof(buf));
			n = read(fd, buf, sizeof(buf));

			if ( n > 0 ) {
				u16 crc;

				printf("Received size %d\n", n);
				hex(n);
				
				if(crc_check( buf, n))
					printf("Okay\n");
				else
					printf("Not okay\n");
				
			}
			else if( n < 0 )
				perror("Read");
			else
				printf("Read Timeout\n");

		}
	}

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

	rs485.c_cflag |= (CLOCAL | CREAD);
	rs485.c_cflag &= ~CSIZE;
	rs485.c_cflag |= CS8;
	rs485.c_cflag &= ~PARENB;
	rs485.c_cflag &= ~CSTOPB;
	rs485.c_iflag = IGNPAR;
	rs485.c_oflag = 0;
	rs485.c_lflag = 0;

	tcsetattr(fd, TCSANOW, &rs485);
}

// ---------- MODBUS Request Frame Setup ----------
void modbus_req(s32 si, u16 sa, u16 qty)
{
	u16 crc;
	sa-=1;
	// ----------- Build Modbus RTU Request -----------
	msg[0] = si;                    // Slave ID
	msg[1] = 0x03;                    // Function Code (Read Holding Registers)
	msg[2] = (sa >> 8) & 0x00FF;
	msg[3] = (sa & 0xFF);
	msg[4] = (qty >> 8) & 0x00FF;
	msg[5] = (qty & 0xFF);
	// Compute CRC dynamically
	crc = modbus_crc(msg, 6);
	msg[6] = (crc & 0x00FF);              // CRC Low byte
	msg[7] = (crc >> 8) & 0x00FF;       // CRC High byte

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

// ---------- Function to Set FD to Read from Device ----------
void read_RS485(void) 
{
	tcdrain(fd);	// Wait until all data is Transmitted
	control_rts(fd, 0);	// Enable receiving

	FD_ZERO(&read_fds);	// clear the read fds
	FD_SET( fd, &read_fds);	// adding fd to read_fds to monitor incoming message
	timeout.tv_sec = 2;	// 2 seconds timeout
	timeout.tv_usec = 0;	

	n = 0;
	n = select( fd+1, &read_fds, NULL, NULL, &timeout);
}

// ---------- Function to check crc of received frame ----------
int crc_check(u8 *buf, s32 n)
{
	int crc;	// Calculated CRC
	u16 rcrc;	// Received CRC

	crc = modbus_crc( buf, n-2);
	rcrc = buf[n-2] & 0xFF;
	rcrc |= ((buf[n-1] & 0xFF) << 8);
	
	if( crc == rcrc )
		return 1;
	else 
		return 0;
}
