#include "http_helpers.h"

char *get_header(const char *headers, const char *key) {
    char *headers_copy = strdup(headers);
    if (headers_copy == NULL) {
        return NULL;
    }
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
            if (value == NULL) {
                free(headers_copy);
                return NULL;
            }
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
char *get_body(const void *request) {
    const char *body_start = strstr(request, "\r\n\r\n");
    if (body_start) {
        body_start += 4; // Move past the "\r\n\r\n"
        return strdup(body_start);
    }
    return NULL; // No body found
}

struct Req_Body parse_request_body(const void *request) {
    struct Req_Body body = {0};

    void *body_raw = get_body(request);
    if (body_raw) {
        body.content = body_raw;
        body.length = strlen(body_raw);
        body.content_type = get_header(request, "Content-Type");
    }
    return body;
}

char *get_boundary(const char *content_type) {
    char *boundary_var = "boundary=";
    char *boundary_definition_start = strstr(content_type, boundary_var);

    if (boundary_definition_start == NULL) {
        return NULL;
    }

    char *value = strstr(boundary_definition_start, "=") + 1; // Get the value after '='
    if (value == NULL) {
        return NULL;
    }

    char *value_copy = strdup(value);

    char *boundary_value = malloc(strlen(value_copy) + 3); // +3 for the two dashes and null terminator
    if (boundary_value == NULL) {
        free(value_copy);
        return NULL;
    }
    strcpy(boundary_value, "--");
    strcat(boundary_value, value_copy);
    free(value_copy);

    return boundary_value;
}


void parse_multipart_form_data(struct Req_Body* body) {
    char *boundary = get_boundary(body->content_type);
    if (boundary == NULL) {
        perror("No boundary found in Content-Type\n");
        return;
    }
    char *form_end_boundary = (char *)malloc(strlen(boundary) + 3); // +3 for the two dashes and null terminator
    if (form_end_boundary == NULL) {
        perror("Failed to allocate memory for form_end_boundary\n");
        free(boundary);
        return;
    }
    strcpy(form_end_boundary, boundary);
    strcat(form_end_boundary, "--");

    char *body_copy = extract_substr(body->content, boundary, form_end_boundary, false, true);
    if (body_copy == NULL) {
        perror("Failed to copy body content\n");
        free(boundary);
        free(form_end_boundary);
        return;
    }
    char *start = body_copy;

    int new_max_size = body->length;
    body->length = 0;
    free(body->content);

    body->content = malloc(new_max_size);
    if (body->content == NULL) {
        perror("Error reallocating space");
        free(boundary);
        free(form_end_boundary);
        free(start);
        return;
    }
    memset(body->content, 0, new_max_size); // Clear new content buffer

    bool is_last_part = false;

    do
    {
        /*
            At this point body_copy contains something like this:
            1 [ \r\n\ ]
            2 [ part 1 headers, such as Content-Disposition and Content-Type]
            3 [ \r\n\r\n ]
            4 [ data]
            5 [ \r\n ]
            6 [ end boundary ]
            7 [ part 2 headers ... ]
            8 [ ... ]
            This extracts the first part, from point 2 (we exclude the first \r\n),
            up until and including the boundary, so this_part contains { 2, 3, 4, 5, 6 }
        */
        char *this_part = extract_substr(body_copy, "\r\n", boundary, false, true);
        if (this_part == NULL) {
            break;
        }

        /*
            Extract the data portion of the part
            3 [ \r\n\r\n ]
            4 [ data]
            5 [ \r\n ]
            6 [ end boundary ]
            Data is always after a double CRLF and before a single CRLF followed by the boundary
        */
        char *part_content_start = extract_substr(this_part, "\r\n\r\n", boundary, false, false);
        size_t part_content_length = strlen(part_content_start);
    
        struct Part *new_part = malloc(sizeof(struct Part));
        if (!new_part) {
            perror("Failed to allocate memory for new_part\n");
            free(this_part);
            free(part_content_start);
            break;
        }
        memset(new_part, 0, sizeof(struct Part));

        set_part_content_disposition(new_part, this_part);
        set_part_form_data_name(new_part);
        set_part_file_name(new_part);
        set_part_content_type(new_part, this_part);
        set_part_content(new_part, part_content_start, part_content_length);
       
        // Append part info to body content
        strncat(body->content, new_part->form_data_name, new_part->fdn_length);
        strcat(body->content, ":\n");
        strncat(body->content, new_part->part_content, new_part->pc_length);
        strcat(body->content, "\n---\n");
        body->length = strlen(body->content);

        free_part(new_part);
        free(this_part);
        free(part_content_start);

        /*
        If the distance to the next form_end_boundary is equal to the distance to the boundary,
        then this is the last part.
        Example:
          Case 1                     |    Case 2
          ___________________________|_____________________
        ----form boundary            |  ----form boundary
            part 1                   |      part 1  
        ----form boundary            |  ----form end boundary--
            part 2                   |
        ----form end boundary--      |
        */
        is_last_part = (strlen(strstr(body_copy, boundary)) == strlen(strstr(body_copy, form_end_boundary)));

        if (is_last_part) {
            break; // Exit loop if last part
        }

        body_copy = strstr(body_copy, boundary);
        if (body_copy == NULL) {
            break; // No more boundaries found
        }
        body_copy += strlen(boundary);
    } while (!is_last_part);
    

    free(start);
    free(form_end_boundary);
    free(boundary);
}

bool is_valid_http_version(const char *version) {
    return (strcmp(version, HTTP_V_1_1) == 0 || strcmp(version, HTTP_V_1_0) == 0);
}

bool is_text_based_mime_type(char *content_type) {
    
    if (strcmp(content_type, MIME_JSON) == 0) return true;
    if (strcmp(content_type, MIME_TEXT_PLAIN) == 0) return true;
    if (strcmp(content_type, MIME_TEXT_HTML) == 0) return true;
    if (strcmp(content_type, MIME_TEXT_JS) == 0) return true;
    if (strcmp(content_type, MIME_TEXT_CSS) == 0) return true;
    if (strcmp(content_type, MIME_URLENCODED) == 0) return true;
    return false;
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

    free(content_length_str);
    free(request_copy);
    return headers;
}

void free_part(struct Part *part) {
    if (part) {
        free(part->content_disposition);
        free(part->form_data_name);
        free(part->content_type);
        free(part->part_content);
        free(part->file_name);
        free(part);
    }
}

void set_part_content_disposition(struct Part *part, const char* part_header) {

    char *part_content_disposition = strstr(part_header, "Content-Disposition: ");
    part_content_disposition += strlen("Content-Disposition: ");
    char *part_content_disposition_copy = strdup(part_content_disposition);
    part_content_disposition = strtok(part_content_disposition_copy, "\r\n");
    part->content_disposition = strdup(part_content_disposition);
    part->cd_length = strlen(part->content_disposition);
    free(part_content_disposition_copy);
}

void set_part_form_data_name(struct Part *part) {
    if (part->content_disposition == NULL) {
        perror("Part Content-Disposition is NULL");
        return;
    }

    char *name_start = strstr(part->content_disposition, "name=\"");
    if (name_start == NULL) {
        part->form_data_name = NULL;
        part->fdn_length = 0;
        return;
    }
    
    char *form_data_name_start = name_start + 6; // Move past 'name="'
    char *form_data_name_end = strstr(form_data_name_start, "\"");
    if (form_data_name_end == NULL) {
        part->form_data_name = NULL;
        part->fdn_length = 0;
        return;
    }
    
    size_t form_data_name_length = form_data_name_end - form_data_name_start;
    part->form_data_name = strndup(form_data_name_start, form_data_name_length);
    part->fdn_length = strlen(part->form_data_name);
}

void set_part_file_name(struct Part *part) {
    if (part->content_disposition == NULL) {
        perror("Part Content-Disposition is NULL");
        return;
    }

    char *form_file_name_start = strstr(part->content_disposition, "filename=\"");
    if (form_file_name_start == NULL) {
        part->file_name = NULL;
        part->fn_length = 0;
        return;
    }
    form_file_name_start += strlen("filename=\"");
    char *form_file_name_end = strstr(form_file_name_start, "\"");
    size_t form_file_name_length = form_file_name_end - form_file_name_start;
    part->file_name = strndup(form_file_name_start, form_file_name_length);
    part->fn_length = strlen(part->file_name);
}

void set_part_content_type(struct Part *part, const char* part_headers) {
    char *part_content_type = strstr(part_headers, "Content-Type: ");
    if (part_content_type == NULL) {
        part->content_type = strdup("text/plain");
        part->ct_length = strlen(part->content_type);
    } else {
        part_content_type += strlen("Content-Type: ");
        char *part_content_type_copy = strdup(part_content_type);
        part_content_type = strtok(part_content_type_copy, "\r\n");
        part->content_type = strdup(part_content_type);
        part->ct_length = strlen(part->content_type);
        free(part_content_type_copy);
    }
}

void set_part_content(struct Part *part, const void* data, size_t data_len) {
    if (is_text_based_mime_type(part->content_type)) {
        part->part_content = strndup(data, data_len); // Copy string with null terminator
    } else {
        part->part_content = malloc(data_len);
        if (part->part_content != NULL) {
            memcpy(part->part_content, data, data_len); // Copy raw bytes
        }
    }

    part->pc_length = data_len;
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
    }
}

struct Response *build_response(const char *status, const char *content_type, size_t content_length, const void *body) {
    // Allocate memory for Response struct
    struct Response *response = malloc(sizeof(struct Response));
    if (!response) {
        return NULL;
    }
    memset(response, 0, sizeof(struct Response));

    // Build headers
    const char *headers_template = "HTTP/1.1 %s\r\nContent-Type: %s\r\nContent-Length: %zu\r\n\r\n";

    // Calculate size needed for headers
    size_t headers_size = snprintf(NULL, 0, headers_template, status, content_type, content_length);
    response->headers = malloc(headers_size + 1);
    if (!response->headers) {
        free(response);
        return NULL;
    }
    memset(response->headers, 0, headers_size + 1);

    // Fill in headers template
    sprintf(response->headers, headers_template, status, content_type, content_length);

    // Build response struct
    response->headers_length = headers_size;
    response->status = strdup(status);
    response->content_type = strdup(content_type);
    response->content_length = content_length;

    if (content_length == 0) {
        response->body = NULL;
        return response;
    }

    response->body = malloc(content_length);
    if (body) {
        memset(response->body, 0, content_length);
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
