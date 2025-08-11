#include"header.h"

bool modbus_tcp(u8* ip, s32 portNo, s32 slaveID, s)
{	
	// Create new MODBUS TCP FD
	modbus_t *ctx = modbus_new_tcp("192.168.1.102",502);
	
	// If failed to create FD
	if(ctx == NULL)
	{
		fprintf(stderr,"failed to create FD\n");
		return 0;
	}
 
	// Connected to Device
	if(modbus_connect(ctx) == -1)
	{
		fprintf(stderr,"unable to connect to device: %s\n",modbus_strerror(errno));
		return 0;
	}

	// Set slave ID
	modbus_set_slave(ctx,1);

	// Read 2 registers from address 0x0000	
	unsigned short int buffer[2];
	int rc = modbus_read_registers(ctx,0x0000, 2, modbus_tcp_buffer);

	if(rc == -1)
	{
		fprintf(stderr,"Failed to read: %s\n",modbus_strerror(errno));
		modbus_close(ctx);
		modbus_free(ctx);
		return 0;
	}

	printf("buffer[0] = 0x%X\n",modbus_tcp_buffer[0]);
	printf("buffer[1] = 0x%X\n",buffer[1]);

	modbus_close(ctx);
	modbus_free(ctx);

	return 1;
}
