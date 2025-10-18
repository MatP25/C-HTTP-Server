#ifndef HTTP_HELPERS_H
#define HTTP_HELPERS_H
#include <sys/types.h>
#include <errno.h>
#include "http.h"

char *get_header(const char *headers, const char *key);
char *get_body(const char *request);
struct Req_Headers parse_request_headers(const char *request);
struct Req_Body parse_request_body(const char *request);
void free_req_headers(struct Req_Headers *headers);
void free_body_content(struct Req_Body *body);
// char *build_response(const char *status, const char *content_type, size_t content_length, const char *body);
struct Response *build_response(const char *status, const char *content_type, size_t content_length, const void *body);
void free_response(struct Response *response);
char *get_file_name(char *content_disposition_header);
#endif