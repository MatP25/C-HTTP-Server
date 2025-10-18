#include "server_handlers.h"
#include "file_helpers.h"

#define MAX_FILE_PATH_LENGTH 1024

void handle_request(
    struct Req_Headers *req_headers, 
    struct Req_Body *req_body, 
    int client_fd) {

    if (req_headers == NULL || req_body == NULL) {
        printf("Error: Request headers or body is NULL.\n");
        send_400(client_fd, "Bad Request: Missing headers or body", 0);
        return;
    }

    if (strcmp(req_headers->protocol, "HTTP/1.1") != 0) {
        printf("Error: Unsupported HTTP version: %s\n", req_headers->protocol);
        send_505(client_fd);
        return;
    }

    if (strcmp(req_headers->method, "GET") == 0) {
        printf("Handling GET request for path: %s\n", req_headers->uri);
        get_handler(req_headers, client_fd);
        return;
    } else if (strcmp(req_headers->method, "POST") == 0) {
        printf("Handling POST request for path: %s\n", req_headers->uri);
        post_handler(req_headers, req_body, client_fd);
        return;
    }
}

void get_handler(struct Req_Headers *req_headers, int client_fd) {

    if (strcmp(req_headers->uri, "/health") == 0) {
        send_200(client_fd, "Server is OK", MIME_TEXT_PLAIN, 0);
        return;
    }

    char *file_full_name = NULL;
    if (strcmp(req_headers->uri, "/") == 0) {
        file_full_name = "index.html";
    } else {
        file_full_name = req_headers->uri + 1; // Skip leading '/'
    }

    if (strlen(file_full_name) + strlen(HTML_DIR) >= MAX_FILE_PATH_LENGTH) {
        send_400(client_fd, "Bad Request: File name too long", 0);
        return;
    }

    char file_path[MAX_FILE_PATH_LENGTH] = HTML_DIR;
    strncat(file_path, file_full_name, strlen(file_full_name));

    struct file_data *filedata = load_file(file_path);
    if (filedata == NULL) {
        send_404(client_fd);
        return;
    }

    const char *mime_type = get_file_mime_type(file_full_name);

    send_200(client_fd, filedata->data, mime_type, filedata->size);
    file_free(filedata);
}

void post_handler(struct Req_Headers *req_headers, struct Req_Body *req_body, int client_fd) {
    if (strcmp(req_headers->uri, "/upload") == 0) {
        printf("Content Disposition header: %s", req_headers->content_disposition);
        send_201(client_fd, "File uploaded successfully", MIME_TEXT_PLAIN, 0);
    } else {
        send_404(client_fd);
    }
}

 // Handles a new connection, receives a pointer to an integer containing the client_fd
void *handle_connection(void *arg)
{
	int client_fd = *((int *)arg);
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

	handle_request(&req_headers, &body_contents, client_fd);

    free(readBuffer);
    free_body_content(&body_contents);
    free_req_headers(&req_headers);

    close(client_fd);
    return NULL;
}

/**
 * `send()` sends data on the client_fd socket.
 * If successful, returns 0 or greater indicating the number of bytes sent, otherwise
 * returns -1.
*/
// void send_response(int client_fd, char *response, int flags) {
//     printf("Sending response to client_fd: %d\n", client_fd);
//     int bytesSent = write(client_fd, response, strlen(response));

//     if (bytesSent == -1) {
//         perror("Sending response failed");
//     } else {
//         printf("Response sent successfully, bytes sent: %d\n", bytesSent);
//     }
// }

void send_response(struct Response *response, int client_fd) {
    printf("Sending response to client_fd: %d\n", client_fd);
    int headersSent = send(client_fd, response->headers, response->headers_length, 0);

    if (headersSent == -1) {
        perror("Sending response headers failed");
        return;
    }

    int bodySent = send(client_fd, response->body, response->content_length, 0);

    if (bodySent == -1) {
        perror("Sending response body failed");
    } else {
        printf("Response sent successfully, bytes sent: %d\n", headersSent + bodySent);
    }
}

void send_200(int client_fd, const char *body, const char *content_type, size_t content_length) {
    struct Response *response = build_response(STATUS_OK, content_type, content_length, body);
    send_response(response, client_fd);
    free_response(response);
}

void send_201(int client_fd, const char *body, const char *content_type, size_t content_length) {
    struct Response *response = build_response(STATUS_CREATED, content_type, content_length, body);
    send_response(response, client_fd);
    free_response(response);
}

void send_400(int client_fd, const char *body, size_t content_length) {
    struct Response *response = build_response(STATUS_BAD_REQUEST, MIME_TEXT_PLAIN, content_length, body);
    send_response(response, client_fd);
    free_response(response);
}

void send_404(int client_fd) {
    char message[] = "Resource Not Found";
    struct Response *response = build_response(STATUS_NOT_FOUND, MIME_TEXT_PLAIN, strlen(message), message);
    send_response(response, client_fd);
    free_response(response);
}

void send_500(int client_fd) {
    char message[] = "Internal Server Error";
    struct Response *response = build_response(STATUS_INTERNAL_SERVER_ERROR, MIME_TEXT_PLAIN, strlen(message), message);
    send_response(response, client_fd);
    free_response(response);
}

void send_505(int client_fd) {
    char message[] = "HTTP Version Not Supported";
    struct Response *response = build_response(STATUS_HTTP_VERSION_NOT_SUPPORTED, MIME_TEXT_PLAIN, strlen(message), message);
    send_response(response, client_fd);
    free_response(response);
}
