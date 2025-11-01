#ifndef ROUTER_H
#define ROUTER_H
#include "api_response.h"
#include "request.h"

typedef struct {
    api_response_t* (*func)(void* args);
    http_method_t method;
    char path[PATH_SIZE];
} route_t;

typedef struct router_t {
    route_t *route;
    struct router_t *next;
} router_t;

router_t* init_router();
void free_router(router_t* router);
void append_route(router_t* target, route_t* route);
route_t* lookup_route(router_t* router, const char request_path[PATH_SIZE], http_method_t method, char* param_out);

#endif