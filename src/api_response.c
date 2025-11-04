#include "api_response.h"

#include <stdlib.h>
#include <string.h>

void parse_paginated_query(char* query, int* page, int* page_size) {
    char *saveptr1, *saveptr2;
    char* pair = strtok_r(query, "&", &saveptr1);
    while (pair != NULL) {
        char* key = strtok_r(pair, "=", &saveptr2);
        char* value = strtok_r(NULL, "=", &saveptr2);

        if (key && value) {
            if (strcasecmp(key, "page") == 0) {
                *page = atoi(value);
            } else if (strcasecmp(key, "page_size") == 0) {
                *page_size = atoi(value);
            }
        }

        pair = strtok_r(NULL, "&", &saveptr1);
    }
}

char* json_trim(char* str) {
    char* end;
    while (*str == ' ' || *str == '\t' || *str == '\"') str++;

    end = str + strlen(str) - 1;
    while (end > str && (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r' || *end == '\"')) {
        *end = '\0';
        end--;
    }

    return str;
}
