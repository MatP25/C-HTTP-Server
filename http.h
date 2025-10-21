#ifndef HTTP_H
#define HTTP_H

#define HTTP_V_1_1 "HTTP/1.1"
#define HTTP_V_1_0 "HTTP/1.0"

#define STATUS_OK "200 OK"
#define STATUS_CREATED "201 Created"
#define STATUS_NOT_FOUND "404 Not Found"
#define STATUS_BAD_REQUEST "400 Bad Request"
#define STATUS_INTERNAL_SERVER_ERROR "500 Internal Server Error"
#define STATUS_NOT_IMPLEMENTED "501 Not Implemented"
#define STATUS_HTTP_VERSION_NOT_SUPPORTED "505 HTTP Version Not Supported"

#define MIME_TEXT_PLAIN "text/plain"
#define MIME_TEXT_HTML "text/html"
#define MIME_TEXT_CSS "text/css"
#define MIME_TEXT_JS "text/javascript"
#define MIME_OCTET_STREAM "application/octet-stream"
#define MIME_URLENCODED "application/x-www-form-urlencoded"
#define MIME_MULTIPART_FORM "multipart/form-data"
#define MIME_JSON "application/json"
#define MIME_JPEG "image/jpeg"
#define MIME_PNG "image/png"

#define POST_DIR "www/post/"
#define HTML_DIR "www/"

struct Req_Headers
{
    char *method;
    char *uri;
    char *protocol;
    char *host;
    char *user_agent;
    char *accept;
    char *content_type;
    int content_length;
};

struct Req_Body {
    char *content_type;
    void *content;
    int length;
};

struct Response {
    char *status;
    char *content_type;
    size_t content_length;
    char *headers;
    int headers_length;
    void *body;
};

struct Part {
    char *content_disposition;
    size_t cd_length;
    char *form_data_name;
    size_t fdn_length;
    char *file_name;
    size_t fn_length;
    char *content_type;
    size_t ct_length;
    void *part_content;
    size_t pc_length;
};


#endif