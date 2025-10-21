#include "request_handlers.h"
#include "file_helpers.h"

void handle_GET(struct Req_Headers *req_headers, int client_fd) {

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

void handle_POST(struct Req_Headers *req_headers, struct Req_Body *req_body, int client_fd) {
    if (strcmp(req_headers->uri, "/post") == 0) {

        char *is_multipart_form = strstr(req_headers->content_type, MIME_MULTIPART_FORM);
        if (is_multipart_form != NULL) {
            parse_multipart_form_data(req_body);
        } else if (strcmp(req_headers->content_type, MIME_TEXT_PLAIN) != 0) {
            send_501(client_fd);
            return;
        }
        
        char *file_full_name = "file.txt";

        char file_path[MAX_FILE_PATH_LENGTH] = POST_DIR;
        strncat(file_path, file_full_name, strlen(file_full_name));

        int write_result = write_file(file_path, req_body->content, req_body->length);

        if (write_result == -1) {
            send_500(client_fd);
            return;
        }

        send_201(client_fd, "File uploaded successfully", MIME_TEXT_PLAIN, 0);
    } else {
        send_404(client_fd);
    }
}