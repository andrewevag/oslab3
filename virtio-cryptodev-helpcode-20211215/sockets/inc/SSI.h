#ifndef SSH_H
#define SSI_H
#include <stdbool.h>
#include <inttypes.h>
/*
 * An implementation of the SSI function calls in MARC J.ROCHKIND "Programming in UNIX", 
 * only for internet domain.
 */

#define SSI_NAME_SIZE 100

typedef struct{
	bool ssi_server;
	int ssi_fd;
	int port;
	char ssi_name_server[SSI_NAME_SIZE];
} SSI;

/*
 * Opens a socket and either listens as a server or connects to as a client.
 * @param name the address of the server (Should be NULL for server);
 * @param port the port to listen or to connect to.
 * @param server a boolean to specify the behaviour.
 * @param tcp_backlog the number of connections that can wait in queue only needed in server
 * @return a pointer to an SSI structure specifying the connection.
 */
SSI* ssi_open(char* name, uint16_t port, bool server, int tcp_backlog);

/**
 * @brief 
 * Waits for a client to connect if the ssi given belongs to a server.
 * @param ssip the SSI* to created by ssi_open
 * @return int the fd of the client -1 in case of error
 */
int ssi_server_accept(SSI* ssip);

/**
 * @brief 
 * Terminates the connection if one is up and closed the corresponding socket.
 * @param ssip a pointer to SSI structure given by open
 * @return true if connection closed successfully
 * 
 */
bool ssi_close(SSI* ssip);


#endif