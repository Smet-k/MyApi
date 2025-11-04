#include "controllers/positionController.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "config/db.h"
#include "services/positionService.h"

#define PAGE_SIZE 10
#define POSITION_SIZE 256

static Position* parse_position_json(char* json);

api_response_t* select_position(void* args) {
    api_request_t* request_params = (api_request_t*)args;
    sqlite3* db = NULL;

    if (open_database(&db) < 0) {
        fprintf(stderr, "Failed to open database\n");
        return &(api_response_t){.body = "Internal Server Error", .code = 500};
    }
    Position position;

    const int id = atoi(request_params->params);

    if (service_select_position_by_id(db, id, &position) < 0) {
        fprintf(stderr, "Failed to select position from database\n");
        sqlite3_close(db);
        return &(api_response_t){.body = "Bad Request", .code = 400};
    }

    char body[256];
    snprintf(body, sizeof(body),
             "{\"id\": %d, \"title\": \"%s\", \"salary\": \"%d\"}",
             position.id, position.title, position.salary);

    sqlite3_close(db);

    api_response_t* response = &(api_response_t){.body = body, .code = 200};
    return response;
}

api_response_t* select_positions(void* args) {
    api_request_t* request_params = (api_request_t*)args;
    sqlite3* db = NULL;
    if (open_database(&db) < 0) {
        fprintf(stderr, "Failed to open database\n");
        return &(api_response_t){.body = "Internal Server Error", .code = 500};
    }

    int count = 0;
    int page = 1;
    int page_size = PAGE_SIZE;

    parse_paginated_query(request_params->query, &page, &page_size);

    Position* positions = calloc(page_size, sizeof(Position) * page_size);

    if (!positions) {
        fprintf(stderr, "Memory allocation failed for employees\n");
        sqlite3_close(db);
        return &(api_response_t){.body = "Internal Server Error", .code = 500};
    }

    if (service_select_positions_paginated(db, page, page_size, positions, &count) < 0) {
        fprintf(stderr, "Failed to select positions from database\n");
        free(positions);
        sqlite3_close(db);
        return &(api_response_t){.body = "Internal Server Error", .code = 500};
    }

    size_t body_size = page_size * POSITION_SIZE;
    char* body = malloc(body_size);

    if (!body) {
        fprintf(stderr, "Memory allocation failed for response body\n");
        sqlite3_close(db);
        free(positions);
        return &(api_response_t){.body = "Internal Server Error", .code = 500};
    }

    snprintf(body, body_size, "[");

    for (int i = 0; i < page_size && i < count; i++) {
        char buf[POSITION_SIZE];
        snprintf(buf, sizeof(buf),
                 "{\"id\": %d, \"title\": \"%s\", \"salary\": \"%d\"}%s",
                 positions[i].id, positions[i].title, positions[i].salary,
                 (i < page_size - 1 && i < count - 1) ? "," : "");
        strncat(body, buf, body_size - strlen(body) - 1);
    }

    strncat(body, "]", body_size - strlen(body) - 1);
    sqlite3_close(db);

    api_response_t* response = &(api_response_t){.body = body, .code = 200};
    return response;
}

api_response_t* add_position(void* args) {
    api_request_t* request_params = (api_request_t*)args;
    sqlite3* db = NULL;

    if (open_database(&db) < 0) {
        fprintf(stderr, "Failed to open database\n");
        return &(api_response_t){.body = "Internal Server Error", .code = 500};
    }

    Position* position = parse_position_json(request_params->body);

    if (service_add_position(db, position) < 0) {
        fprintf(stderr, "Failed to add position\n");
        return &(api_response_t){.body = "Internal Server Error", .code = 500};
    }

    api_response_t* response = &(api_response_t){.body = "Position created", .code = 201};
    return response;
}
api_response_t* delete_position(void* args) {
    api_request_t* request_params = (api_request_t*)args;
    sqlite3* db = NULL;

    if (open_database(&db) < 0) {
        fprintf(stderr, "Failed to open database\n");
        return &(api_response_t){.body = "Internal Server Error", .code = 500};
    }

    const int id = atoi(request_params->params);

    if (service_delete_position_by_id(db, id) < 0) {
        fprintf(stderr, "Failed to delete position from database\n");
        sqlite3_close(db);
        return &(api_response_t){.body = "Bad Request", .code = 400};
    }

    sqlite3_close(db);

    api_response_t* response = &(api_response_t){.body = "Position deleted", .code = 204};
    return response;
}

api_response_t* update_position(void* args) {
    api_request_t* request_params = (api_request_t*)args;
    sqlite3* db = NULL;

    if (open_database(&db) < 0) {
        fprintf(stderr, "Failed to open database\n");
        return &(api_response_t){.body = "Internal Server Error", .code = 500};
    }

    Position* position = parse_position_json(request_params->body);

    if (position->id <= 0) {
        fprintf(stderr, "No id provided\n");
        return &(api_response_t){.body = "Bad Request", .code = 400};
    }

    if (service_update_position(db, position) < 0) {
        fprintf(stderr, "Position with provided ID doesn't exist\n");
        return &(api_response_t){.body = "Bad Request", .code = 400};
    }

    api_response_t* response = &(api_response_t){.body = "Position updated", .code = 200};
    return response;
}

static Position* parse_position_json(char* json) {
    Position* p = calloc(1, sizeof(Position));
    char *str1, *token, *subtoken;
    char *saveptr1, *saveptr2;

    // Clearing braces
    json += 1;
    json[strlen(json) - 1] = '\0';

    for (str1 = json;; str1 = NULL) {
        token = strtok_r(str1, ",", &saveptr1);
        if (token == NULL)
            break;

        subtoken = json_trim(strtok_r(token, ":", &saveptr2));
        char* value = json_trim(strtok_r(NULL, ":", &saveptr2));

        if (strcmp("id", subtoken) == 0)
            p->id = atoi(value);
        else if (strcmp("title", subtoken) == 0)
            strcpy(p->title, value);
        else if (strcmp("salary", subtoken) == 0)
            p->salary = atoi(value);
    }

    return p;
}