#ifndef C_WEBSERVER_KQUEUE_HTTP_H
#define C_WEBSERVER_KQUEUE_HTTP_H
enum HttpMethod {
    HTTP_METHOD_GET = 1,
    HTTP_METHOD_HEAD,
    HTTP_METHOD_POST,
    HTTP_METHOD_PUT,
    HTTP_METHOD_DELETE,
    HTTP_METHOD_TRACE,
    HTTP_METHOD_OPTIONS,
    HTTP_METHOD_CONNECT,
    HTTP_METHOD_PATCH,
};

enum HttpVersion {
    HTTP_VERSION_0_9 = 1,
    HTTP_VERSION_1_0,
    HTTP_VERSION_1_1,
};

struct MethodTableEntry {
    char *request_line_prefix;
    enum HttpMethod method;
};

struct HttpRequest {
    enum HttpMethod method;
    char *target;
    enum HttpVersion version;
    char *message_body;
};

#endif //C_WEBSERVER_KQUEUE_HTTP_H
