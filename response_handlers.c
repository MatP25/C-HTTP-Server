#include "response_handlers.h"

/**
 * `send()` sends data on the client_fd socket.
 * If successful, returns 0 or greater indicating the number of bytes sent, otherwise
 * returns -1.
*/
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

void send_501(int client_fd) {
    char message[] = "Not Implemented";
    struct Response *response = build_response(STATUS_NOT_IMPLEMENTED, MIME_TEXT_PLAIN, strlen(message), message);
    send_response(response, client_fd);
    free_response(response);
}

void send_505(int client_fd) {
    char message[] = "HTTP Version Not Supported";
    struct Response *response = build_response(STATUS_HTTP_VERSION_NOT_SUPPORTED, MIME_TEXT_PLAIN, strlen(message), message);
    send_response(response, client_fd);
    free_response(response);
}