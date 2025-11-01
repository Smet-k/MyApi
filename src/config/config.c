#include "config/config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAXLINE 128

static char* trim(char* str) {
    char* end;
    while (*str == ' ' || *str == '\t') str++;

    end = str + strlen(str) - 1;
    while (end > str && (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r')) {
        *end = '\0';
        end--;
    }

    return str;
}

int load_config(const char* filename, Config* config) {
    FILE* file = fopen(filename, "r");
    if (!file) return -1;

    char line[MAXLINE];

    while (fgets(line, sizeof(line), file) != NULL) {
        char* trimmed = trim(line);

        char* key = strtok(trimmed, "=");
        char* value = strtok(NULL, "=");

        if (key && value) {
            if (strcmp(key, "port") == 0) {
                config->port = atoi(value);
            } else if (strcmp(key, "threads") == 0) {
                config->threads = atoi(value);
            }
        }
    }
    
    return 0;
}

void config_router_append(Config* cfg, char* path, http_method_t method, void* function){
    route_t* r1 = malloc(sizeof(route_t));
    r1->func = function;
    r1->method = method;
    strcpy(r1->path, path);
    append_route(cfg->router, r1);
}

