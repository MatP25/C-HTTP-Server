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

	/**
	 * Disable output buffering.
	 * This means that the program will not wait to
	 * output to the terminal
	 *
	 * For example, when doing
	 * printf("a");
	 * printf("b");
	 * printf("c");
	 * The program will output a,b, and c immediately after
	 * each print instead of waiting for a newline ('\n')
	 */
	setbuf(stdout, NULL);

	// printf for debugging
	printf("Starting server...\n");

	/**
	 * Creates a variable srv_addr that stores an IPv4 socket address.
	 * .sin_family = AF_INET -> Indicates that the socket address is an IPv4 address
	 * .sin_port = htons(PORT) -> Converts the port number from `Host Byte Order` to
	 * 								`Network Byte Order`
	 * .sin_addr = { htonl(INADDR_ANY) } -> "Sets the IP address to INADDR_ANY, which
	 * 										means the socket will be bound to all available
	 * 										network interfaces on the machine. INADDR_ANY is
	 * 										a constant that represents "any" network interface."
	 * 										- Dave (ChatGPT)
	 *
	 * "The htons() function converts the unsigned short integer hostshort from host byte order to network byte order. "
	 * "The htonl() function converts the unsigned integer hostlong from host byte order to network byte order. "
	 * 		- https://linux.die.net/man/3/htons
	 *
	 * struct sockaddr_in is a struct from in.h that has the following data:
	 * 			sin_port -> Port number
	 * 			sin_addr -> Internet address
	 * 			sin_zero -> Padding to make the structure the same size as sockaddr
	 */
	struct sockaddr_in serv_addr = {
		.sin_family = AF_INET,			 // IPv4
		.sin_port = htons(PORT),		 // `Host Byte Order` to `Network Byte Order` short
		.sin_addr = {htonl(INADDR_ANY)}, // `Host Byte Order` to `Network Byte Order` long
	};

	/**
	 * Creates a socket connection using IPv4 (AF_INET), and TCP (SOCK_STREAM)
	 * Other options include AF_INET6 for IPv6 and SOCK_DGRAM for UDP.
	 *
	 * The last parameter is the protocol, which according to the documentation:
	 *
	 * 		"If the protocol parameter is set to 0, the system selects the default
	 * 		protocol number for the domain and socket type requested."
	 *
	 * I was a bit confused about why is it necessary to specify the protocol twice.
	 * For instance, SOCK_STREAM is already selecting TCP, but the function also
	 * expects IPPROTO_TCP as the third parameter. I couldn't really find an answer
	 * so I assume it is for safety reasons or if you know what you are doing.
	 *
	 * socket() returns a number greater or equal than 0 if the connection was successful
	 * or -1 if an error occurred
	 */
	// Variable to represent the file descriptor (fd) of the server socket
	// Default protocol is 0
	int server_fd = socket(AF_INET, SOCK_STREAM, 0); 
	if (server_fd == -1)
	{
		perror("Socket creation failed");
		exit(EXIT_FAILURE);
	}

	/**
	 * setting REUSE_PORT ensures that we don't run into 'Address already in use' errors
	 */
	int reuse = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0)
	{
		perror("SO_REUSEADDR failed");
		exit(EXIT_FAILURE);
	}

	/**
	 * 		"The bind() function binds a unique local name to the socket with descriptor socket.
	 * 		After calling socket(), a descriptor does not have a name associated with it.
	 * 		However, it does belong to a particular address family as specified when socket()
	 * 		is called. The exact format of a name depends on the address family."
	 *
	 * 	- https://www.ibm.com/docs/en/zos/2.3.0?topic=functions-bind-bind-name-socket
	 *
	 * In other words: when you first create a socket using `socket()` function, the socket is
	 * not yet associated with a specific address or port.
	 *
	 * Params:
	 * -> server_fd is the socket identifier that we created before.
	 * -> serv_addr is the struct that we specified. serv_addr is casted from
	 * sockaddr_in to sockaddr since sockaddr is the generic struct for socket addresses.
	 * -> The size of the socket address structure.
	 *
	 * The function should return 0 if the binding was successful
	 */
	if (bind(server_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) != 0)
	{
		perror("Bind failed");
		exit(EXIT_FAILURE);
	}

	// Maximum length of the queue of pending connections
	int connection_backlog = 5; 

	/**
	 * `listen()` indicates that the server socket is ready to accept incoming connections
	 * returns 0 if connection was successful
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
		// Variable of type struct sockaddr_in to store the client address
		struct sockaddr_in client_addr; 
		
		// Variable to store the length of the struct client_addr
		socklen_t client_addr_len = sizeof(client_addr);

		// Variable to represent the file descriptor (fd) of the client socket
		int *client_fd = malloc(sizeof(int));
		*client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len); 

		if (*client_fd == -1)
		{
			perror("Failed to connect to client");
			free(client_fd); // Free the malloc'd pointer on error
		} else {
			printf("Client connected: %d\n", *client_fd);
			
			pthread_t thread_pid;
			// Create a new thread that runs the handle_connection function
			// and passes the client_fd as an argument
			// handle_connection closes it's own socket once it finishes
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
