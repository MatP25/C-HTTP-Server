#ifndef REQUEST_HANDLERS_H
#define REQUEST_HANDLERS_H

#include "response_handlers.h"

#define MAX_FILE_PATH_LENGTH 1024

void handle_GET(struct Req_Headers *req_headers, int client_fd);
void handle_POST(struct Req_Headers *req_headers, struct Req_Body *req_body, int client_fd);

#endif