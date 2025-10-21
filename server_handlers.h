#ifndef SERVER_HANDLERS_H
#define SERVER_HANDLERS_H

#include "response_handlers.h"
#include "request_handlers.h"

void router(struct Req_Headers *req_headers, struct Req_Body *request_body, int client_fd);
void *handle_connection(void *arg);

#endif