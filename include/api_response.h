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

typedef struct {
    int page;
    int page_size;
    int count;
    int total_count;
} paginated_request_t;

void parse_paginated_query(char* query, int* page, int* page_size);
void parse_paginated_with_search(char* query, int* page, int* page_size, char* search);
char* json_trim(char* str);

#endif
