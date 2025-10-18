#ifndef HTTP_H
#define HTTP_H

#define STATUS_OK "200 OK"
#define STATUS_CREATED "201 Created"
#define STATUS_NOT_FOUND "404 Not Found"
#define STATUS_BAD_REQUEST "400 Bad Request"
#define STATUS_INTERNAL_SERVER_ERROR "500 Internal Server Error"
#define STATUS_HTTP_VERSION_NOT_SUPPORTED "505 HTTP Version Not Supported"

#define MIME_TEXT_PLAIN "text/plain"
#define MIME_TEXT_HTML "text/html"
#define MIME_TEXT_CSS "text/css"
#define MIME_TEXT_JS "text/javascript"
#define MIME_OCTET_STREAM "application/octet-stream"
#define MIME_JSON "application/json"
#define MIME_JPEG "image/jpeg"
#define MIME_PNG "image/png"

#define UPLOAD_DIR "uploads/"
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
    char *content_disposition;
    int content_length;
};

struct Req_Body {
    char *content_type;
    char *content;
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


#endif