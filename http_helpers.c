#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>  
#include <sys/socket.h>
#include <unistd.h>

#include "http_helpers.h"

char *get_header(const char *headers, const char *key) {
    char *headers_copy = strdup(headers);
    char *line = strtok(headers_copy, "\r\n");
    size_t key_len = strlen(key);
    
    if (strcmp(key, "Method") == 0) {
        char *extracted = strtok(headers_copy, " ");
        char *value = strdup(extracted);
        free(headers_copy);
        return value;
    }

    if (strcmp(key, "Path") == 0) {
        strtok(headers_copy, " "); // Skip method
        char *extracted = strtok(NULL, " ");
        char *value = strdup(extracted);
        free(headers_copy);
        return value;
    }

    if (strcmp(key, "Protocol") == 0) {
        strtok(headers_copy, " "); // Skip method
        strtok(NULL, " ");         // Skip path
        char *extracted = strtok(NULL, "\r\n");
        char *value = strdup(extracted);
        free(headers_copy);
        return value;
    }

    while (line != NULL) {
        if (strncmp(line, key, key_len) == 0 && line[key_len] == ':') {
            char *value_start = line + key_len + 1; // Skip the colon
            while (*value_start == ' ') value_start++; // Skip leading spaces
            char *value = strdup(value_start);
            free(headers_copy);
            return value;
        }
        line = strtok(NULL, "\r\n");
    }
    
    free(headers_copy);
    return NULL; // Key not found
}


/**
 * \r\n\r\n represents the CRLF:
 * 		CR = Carriage Return (\r, 0x0D in hexadecimal, 13 in decimal) — moves the cursor to the beginning
 * 		of the line without advancing to the next line.
 *
 * 		LF = Line Feed (\n, 0x0A in hexadecimal, 10 in decimal) — moves the cursor down to the next line
 * 		without returning to the beginning of the line.
 *
 * - https://developer.mozilla.org/en-US/docs/Glossary/CRLF
*/
char *get_body(const char *request) {
    const char *body_start = strstr(request, "\r\n\r\n");
    if (body_start) {
        body_start += 4; // Move past the "\r\n\r\n"
        return strdup(body_start);
    }
    return NULL; // No body found
}

struct Req_Body parse_request_body(const char *request) {
    struct Req_Body body = {0};

    char *body_raw = get_body(request);
    if (body_raw) {
        body.content = body_raw;
        body.length = strlen(body_raw);
        body.content_type = get_header(request, "Content-Type");
    }
    return body;
}

struct Req_Headers parse_request_headers(const char *request) {
    struct Req_Headers headers = {0};
    char *request_copy = strdup(request);
    
    headers.method = get_header(request_copy, "Method");
    headers.uri = get_header(request_copy, "Path");
    headers.protocol = get_header(request_copy, "Protocol");
    headers.host = get_header(request_copy, "Host");
    headers.user_agent = get_header(request_copy, "User-Agent");
    headers.accept = get_header(request_copy, "Accept");
    headers.content_type = get_header(request_copy, "Content-Type");
    char *content_length_str = get_header(request_copy, "Content-Length");
    headers.content_length = content_length_str ? atoi(content_length_str) : 0;
    headers.content_disposition = get_header(request_copy, "Content-Disposition");

    free(request_copy);
    return headers;
}

void free_body_content(struct Req_Body *body) {
    if (body) {
        free(body->content);
        free(body->content_type);
    }
}

void free_req_headers(struct Req_Headers *headers) {
    if (headers) {
        free(headers->method);
        free(headers->uri);
        free(headers->protocol);
        free(headers->host);
        free(headers->user_agent);
        free(headers->accept);
        free(headers->content_type);
        free(headers->content_disposition);
    }
}


// char *build_response(const char *status, const char *content_type, size_t content_length, const char *body) {
//     const char *template = "HTTP/1.1 %s\r\nContent-Type: %s\r\nContent-Length: %zu\r\n\r\n%s";

//     size_t response_size = snprintf(NULL, 0, template, status, content_type, content_length, body);

//     char *response = malloc(response_size + 1);

//     if (response) {
//         sprintf(response, template, status, content_type, content_length, body);
//     }

//     return response;
// }

struct Response *build_response(const char *status, const char *content_type, size_t content_length, const void *body) {
    // Allocate memory for Response struct
    struct Response *response = malloc(sizeof(struct Response));
    if (!response) {
        return NULL;
    }

    // Build headers
    const char *headers_template = "HTTP/1.1 %s\r\nContent-Type: %s\r\nContent-Length: %zu\r\n\r\n";

    // Calculate size needed for headers
    size_t headers_size = snprintf(NULL, 0, headers_template, status, content_type, content_length);
    response->headers = malloc(headers_size + 1);
    if (!response->headers) {
        free(response);
        return NULL;
    }

    // Fill in headers template
    sprintf(response->headers, headers_template, status, content_type, content_length);

    // Build response struct
    response->headers_length = headers_size;
    response->status = strdup(status);
    response->content_type = strdup(content_type);
    response->content_length = content_length;
    response->body = malloc(content_length);
    if (body) {
        // Use memcpy to copy body content so that it works for binary data as well
        memcpy(response->body, body, content_length);
    } else {
        response->body = NULL;
    }

    return response;
}

void free_response(struct Response *response) {
    if (response) {
        free(response->status);
        free(response->content_type);
        free(response->headers);
        free(response->body);
        free(response);
    }
}
