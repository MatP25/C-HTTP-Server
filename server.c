#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>
#include "server_handlers.h"


int main(int argc, char **argv)
{
	printf("args count: %d\n", argc);
	if (argc < 2) {
		printf("Missing argument, please provide the port number");
		exit(EXIT_FAILURE);
	}
	
	const char *portAsChar = argv[1];
	const int PORT = atoi(portAsChar);
	if (PORT <= 0 || PORT > 65535) {
		printf("Invalid port number: %s\n", portAsChar);
		exit(EXIT_FAILURE);
	}

	setbuf(stdout, NULL);
	printf("Starting server...\n");

	/*
		sockaddr_in : https://man7.org/linux/man-pages/man3/sockaddr.3type.html
		sin_family  : Address family (AF_INET for IPv4) (AF_INET6 for IPv6)
		sin_port    : Port number
		sin_addr    : IP address

		htons       : https://man7.org/linux/man-pages/man3/htons.3p.html
		Converts unsigned short integer (16-bit) from host byte order to network byte order.

		htonl       : https://man7.org/linux/man-pages/man3/htonl.3p.html
		Converts unsigned long integer (32-bit) from host byte order to network byte order.

		The htons function converts a 16-bit unsigned integer (short) from host byte order to network byte order. 
		Network byte order is always big-endian. This ensures a consistent interpretation across different systems.
		Similarly, the htonl function converts a 32-bit unsigned integer (long) from host byte order to network byte order.

		INADDR_ANY is a special address constant used to indicate that a socket should bind to all network interfaces on a machine. 
		When a server application binds to INADDR_ANY, it can accept connections and receive data from any IP address that the computer has, including its loopback address 127.0.0.1
	*/
	struct sockaddr_in serv_addr = {
		.sin_family = AF_INET,			
		.sin_port = htons(PORT),		 
		.sin_addr = {htonl(INADDR_ANY)}, 
	};

	/*
		socket     : https://man7.org/linux/man-pages/man2/socket.2.html
		argument 1 : domain   --> AF_INET for IPv4
		argument 2 : type     --> SOCK_STREAM for bidirectional stream
		argument 3 : protocol --> 0 to select default protocol for given domain and type (TCP for AF_INET and SOCK_STREAM)
		returns    : file descriptor for the server socket or -1 on error
	*/
	int server_fd = socket(AF_INET, SOCK_STREAM, 0); 
	if (server_fd == -1)
	{
		perror("Socket creation failed");
		exit(EXIT_FAILURE);
	}

	/*
		setsockopt : https://man7.org/linux/man-pages/man3/setsockopt.3p.html
		argument 1 : file descriptor for locating the socket
		argument 2 : level  --> SOL_SOCKET to manipulate options at the sockets API level
		argument 3 : option --> SO_REUSEADDR to allow reuse of local addresses
		argument 4 : pointer to option value
		argument 5 : size of the option value
		returns    : 0 on success, -1 on error
	*/
	int reuse = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0)
	{
		perror("SO_REUSEADDR failed");
		exit(EXIT_FAILURE);
	}

	/*
		bind : https://man7.org/linux/man-pages/man2/bind.2.html
		argument 1 : server_fd  --->  file descriptor of the socket to be bound
		argument 2 : serv_addr  --->  pointer to the sockaddr structure (casted from sockaddr_in to sockaddr)
		argument 3 : size of the sockaddr structure
		returns    : 0 on success, -1 on error

		assigns a name to a socket identified by socket file descriptor
	*/
	if (bind(server_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) != 0)
	{
		perror("Bind failed");
		exit(EXIT_FAILURE);
	}

	// Maximum length of the queue of pending connections
	int connection_backlog = 5; 

	/*
		listen : https://man7.org/linux/man-pages/man2/listen.2.html
		argument 1 : server_fd           --->  file descriptor of the socket to be marked as a passive socket
		argument 2 : connection_backlog  --->  maximum length of the queue of pending connections
		returns    : 0 on success, -1 on error

		a passive socket is will be used to accept incoming connections
		connection_backlog defines the maximum length to which the queue of pending connections for sockfd may grow
	*/
	if (listen(server_fd, connection_backlog) != 0)
	{
		perror("Listen failed");
		exit(EXIT_FAILURE);
	}
	printf("Server is listening on PORT %d...\n", PORT);

	while (1)
	{
		printf("Waiting for a new connection...\n");
		struct sockaddr_in client_addr; // Stores the client address
		socklen_t cl_addr_len = sizeof(client_addr);

		// Variable to represent the file descriptor (fd) of the client socket
		int *client_fd = malloc(sizeof(int));
		/*
			accept : https://man7.org/linux/man-pages/man2/accept.2.html
			argument 1 : server_f     --->  file descriptor of the listening socket
			argument 2 : client_addr  --->  pointer to a sockaddr structure to store the address of the connecting entity
			argument 3 : cl_addr_len  --->  pointer to a socklen_t variable that initially contains the size of client_addr structure
			returns    : file descriptor for the accepted socket or -1 on error

			It extracts the first connection request on the queue of pending connections for the listening socket, 
			sockfd, creates a new connected socket, 
			and returns a new file descriptor referring to that socket.  
			The newly created socket is not in the listening state.  
			The original socket sockfd is unaffected by this call.
		*/
		*client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &cl_addr_len); 

		if (*client_fd == -1)
		{
			perror("Failed to connect to client");
			free(client_fd); // Free the malloc'd pointer on error
		} else {
			printf("Client connected: %d\n", *client_fd);
			
			pthread_t thread_pid;
			/*
				Create a new thread that runs the handle_connection function, 
				and passes the client_fd as an argument
				Handle_connection closes it's own socket once it finishes
			*/
			int thread_result = pthread_create(&thread_pid, NULL, handle_connection, (void *)client_fd);
			if (thread_result != 0) {
				perror("Failed to create thread");
				close(*client_fd);
				free(client_fd); // Free the malloc'd pointer on pthread_create error
			} else {
				// Detach the thread to allow it to run independently
				pthread_detach(thread_pid);
			}	
		}
	}

	printf("Shutting down server...\n");
	close(server_fd);
	return 0;
}
