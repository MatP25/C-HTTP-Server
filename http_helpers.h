#ifndef HTTP_HELPERS_H
#define HTTP_HELPERS_H

#include "includes.h"
#include "other_helpers.h"

char *get_header(const char *headers, const char *key);
char *get_body(const void *request);
struct Req_Headers parse_request_headers(const char *request);
struct Req_Body parse_request_body(const void *request);
void free_req_headers(struct Req_Headers *headers);
void free_body_content(struct Req_Body *body);
char *get_boundary(const char *content_type);
void parse_multipart_form_data(struct Req_Body* body);
void free_part(struct Part *part);
bool is_text_based_mime_type(char *content_type);
bool is_valid_http_version(const char *version);
struct Response *build_response(const char *status, const char *content_type, size_t content_length, const void *body);
void free_response(struct Response *response);

void set_part_content_disposition(struct Part *part, const char* part_header);
void set_part_form_data_name(struct Part *part);
void set_part_file_name(struct Part *part);
void set_part_content_type(struct Part *part, const char* part_headers);
void set_part_content(struct Part *part, const void* data, size_t data_len);


#endif