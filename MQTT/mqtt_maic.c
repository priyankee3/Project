#include"header.h"
int main()
{
	const s8 *hostname = "192.168.1.158";
	s32 port = 1883;
	s32 status = 0;
	const s8 *exchange = "pi_data";
	const s8 *routingkey = "test/topic";
	const s8 *message = "Hello from Pi";
	amqp_socket_t *socket = NULL;
	amqp_connection_state_t conn;
	
	// Create a connection  
	conn = amqp_new_connection();
	socket = amqp_tcp_socket_new(conn);
	if(!socket)
	{
		fprintf(stderr,"TCP Socket Status:");
		return 1;
	}

	status = amqp_socket_open(socket, hostname, port);
	if(status)
	{
		fprintf(stderr,"TCP Socket Open Status:");
		return 1;
	}

	// Login 
	if(amqp_login(conn, "/", 0,131072, 0, AMQP_SASL_METHOD_PLAIN,"ee3", "Pump@2021").reply_type != AMQP_RESPONSE_NORMAL)
	{
		fprintf(stderr,"Login Failed:");
		return 1;
	}

	// Open channel
	amqp_channel_open(conn, 1);
	amqp_get_rpc_reply(conn);


	// Publish message
	amqp_basic_publish(conn, 1, amqp_cstring_bytes(exchange), amqp_cstring_bytes(routingkey), 0, 0, NULL, amqp_cstring_bytes(message));

	// Close
	amqp_channel_close(conn, 1, AMQP_REPLY_SUCCESS);
	amqp_connection_close(conn, AMQP_REPLY_SUCCESS);
	amqp_destroy_connection(conn);

	return 0;
}
