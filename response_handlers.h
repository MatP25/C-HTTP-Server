#ifndef RESPONSE_HANDLERS_H
#define RESPONSE_HANDLERS_H

#include "includes.h"
#include "http_helpers.h"

void send_response(struct Response *response, int client_fd);
void send_200(int client_fd, const char *body, const char *content_type, size_t content_length);
void send_201(int client_fd, const char *body, const char *content_type, size_t content_length);
void send_400(int client_fd, const char *body, size_t content_length);
void send_404(int client_fd);
void send_500(int client_fd);
void send_501(int client_fd);
void send_505(int client_fd);

#endif