#ifndef API_RESPONSE_H
#define API_RESPONSE_H
typedef struct {
    char* body;
    int code;
} api_response_t;

typedef struct {
    char* params;
    char* query;
    char* body;
} api_request_t;

#endif
