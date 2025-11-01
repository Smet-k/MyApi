#include "config/router.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

router_t* init_router() {
    router_t* router = malloc(sizeof(router_t));
    if (!router) {
        perror("malloc failed");
        exit(1);
    }
    router->route = NULL;
    router->next = NULL;
    return router;
}

void free_router(router_t* router) {
    router_t* current = router;
    while (current) {
        router_t* next = current->next;
        if (current->route) {
            free(current->route);
        }
        free(current);
        current = next;
    }
}

void append_route(router_t* target, route_t* route) {
    router_t* newRoute = init_router();
    newRoute->route = route;

    if (target->route == NULL) {
        target->route = route;
        return;
    }

    router_t* cursor = target;
    while (cursor->next != NULL)
        cursor = cursor->next;

    cursor->next = newRoute;
}


route_t* lookup_route(router_t* router, const char request_path[PATH_SIZE], http_method_t method, char* param_out) {
    router_t* current = router;

    while (current) {
        route_t* route = current->route;

        if (route->method != method) {
            current = current->next;
            continue;
        }

        char route_path[PATH_SIZE];
        char req_path[PATH_SIZE];

        strncpy(route_path, route->path, PATH_SIZE);
        strncpy(req_path, request_path, PATH_SIZE);

        int matched = 1;
        char* route_ctx;
        char* req_ctx;
        char* route_tok = strtok_r(route_path, "/", &route_ctx);
        char* req_tok   = strtok_r(req_path, "/", &req_ctx);

        while (route_tok && req_tok) {
            if (route_tok[0] == ':') {
                if (param_out)
                    strcpy(param_out, req_tok);
            } else if (strcmp(route_tok, req_tok) != 0) {
                matched = 0;
                break;
            }

            route_tok = strtok_r(NULL, "/", &route_ctx);
            req_tok   = strtok_r(NULL, "/", &req_ctx);
        }


        if (matched && !route_tok && !req_tok) {
            return route;
        }

        current = current->next;
    }
    return NULL;
}