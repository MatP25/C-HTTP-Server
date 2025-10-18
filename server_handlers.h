#ifndef SERVER_HANDLERS_H
#define SERVER_HANDLERS_H
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <time.h>
#include <fcntl.h>
#include "http_helpers.h"
#include "http.h"

void handle_request(struct Req_Headers *req_headers, struct Req_Body *request_body, int client_fd);
void *handle_connection(void *arg);
// void send_response(int client_fd, char *response, int flags);
void send_response(struct Response *response, int client_fd);
void get_handler(struct Req_Headers *req_headers, int client_fd);
void post_handler(struct Req_Headers *req_headers, struct Req_Body *req_body, int client_fd);

void send_200(int client_fd, const char *body, const char *content_type, size_t content_length);
void send_201(int client_fd, const char *body, const char *content_type, size_t content_length);
void send_400(int client_fd, const char *body, size_t content_length);
void send_404(int client_fd);
void send_500(int client_fd);
void send_505(int client_fd);

#endif