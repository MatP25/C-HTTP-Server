#include "server_handlers.h"
#include "http_helpers.h"

void router(
    struct Req_Headers *req_headers, 
    struct Req_Body *req_body, 
    int client_fd) {

    if (req_headers == NULL || req_body == NULL) {
        printf("Error: Request headers or body is NULL.\n");
        send_400(client_fd, "Bad Request: Missing headers or body", 0);
        return;
    }

    if (!is_valid_http_version(req_headers->protocol)) {
        printf("Error: Unsupported HTTP version: %s\n", req_headers->protocol);
        send_505(client_fd);
        return;
    }

    if (strcmp(req_headers->method, "GET") == 0) {
        printf("Handling GET request for path: %s\n", req_headers->uri);
        handle_GET(req_headers, client_fd);
        return;
    } else if (strcmp(req_headers->method, "POST") == 0) {
        printf("Handling POST request for path: %s\n", req_headers->uri);
        handle_POST(req_headers, req_body, client_fd);
        return;
    } else {
        printf("Error: Unsupported HTTP method: %s\n", req_headers->method);
        send_501(client_fd);
        return;
    }
}

 // Handles a new connection, receives a pointer to an integer containing the client_fd
void *handle_connection(void *arg)
{
	int client_fd = *((int *)arg);
	free(arg); // Free the malloc'd client_fd_ptr from server.c
	printf("Started new connection with client: %d\n", client_fd);
	printf("\n");

	/**
	 * `recv()` receives data on the client_fd socket and stores it in the readBuffer buffer.
	 * If successful, returns the length of the message or datagram in bytes, otherwise
	 * returns -1.
	 */
	const int bufferSize = 1024 * 1024 * 20; // 20MB
	char* readBuffer = malloc(bufferSize);

    if (readBuffer == NULL) {
        perror("Failed to allocate memory for readBuffer");
        close(client_fd);
        return NULL;
    }

    memset(readBuffer, 0, bufferSize); // Clear buffer
	int bytesReceived = recv(client_fd, readBuffer, bufferSize, 0);
    

    // For debugging only
    // printf("-------------------\n");
    // printf("Client_fd: %d, bytesReceived: %d at timestamp: %ld\n", client_fd, bytesReceived, time(NULL));
    // printf("Received request:\n%s\n", readBuffer);
    // printf("-------------------\n");

	if (bytesReceived == -1)
	{
		perror("Receiving failed");
		close(client_fd);
		return NULL;
	}

	struct Req_Headers req_headers = parse_request_headers(readBuffer);
	struct Req_Body body_contents = parse_request_body(readBuffer);

	router(&req_headers, &body_contents, client_fd);

    free(readBuffer);
    free_body_content(&body_contents);
    free_req_headers(&req_headers);

    close(client_fd);
    printf("Closed connection with client: %d\n", client_fd);
    return NULL;
}
