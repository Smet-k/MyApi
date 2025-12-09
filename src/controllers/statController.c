#include "controllers/statController.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "config/db.h"
#include "services/statService.h"

#define PAGE_SIZE 10
#define STAT_SIZE 256

static void json_remove_spaces(char *s);
static Stat* parse_stat_json(char* json);

api_response_t* select_stat(void* args){
    api_request_t* request_params = (api_request_t*)args;
    sqlite3* db = NULL;

    if (open_database(&db) < 0) {
        fprintf(stderr, "Failed to open databse\n");
        return &(api_response_t) {.reason="Internal Server Error", .code=500};
    }
    Stat stat;

    const int id = atoi(request_params->params);

    if (service_select_stat_by_id(db, id, &stat) < 0) {
        fprintf(stderr, "Failed to select employee from database\n");
        sqlite3_close(db);
        return &(api_response_t){.reason="Bad Request", .code=400};
    }

    char body[STAT_SIZE + 32];
    snprintf(body, sizeof(body),
        "{\"id\": %d, \"date\": \"%s\", \"time\": \"%s\", \"clock_amount\": %d, \"employee_id\": %d }",
        stat.id, stat.date, stat.time, stat.clock_amount, stat.employee_id);

    sqlite3_close(db);

    api_response_t* response = &(api_response_t){.body = body, .code = 200, .reason="OK"};
    return response;
}

api_response_t* select_stats(void* args){
    api_request_t* request_param = (api_request_t*)args;
    sqlite3* db = NULL;

    if(open_database(&db) < 0) {
        fprintf(stderr, "Failed to open database\n");
        return &(api_response_t){.reason="Internal Server Error", .code=500};
    }

    int page = 1;
    int page_size = PAGE_SIZE;

    parse_paginated_query(request_param->query, &page, &page_size);
    paginated_request_t request = {.page=page, page_size=page_size};

    Stat* stats = calloc(page_size, sizeof(Stat) * page_size);

    if (!stats) {
        fprintf(stderr, "Memory allocation failed for stats\n");
        sqlite3_close(db);
        return &(api_response_t){.reason="Internal Server Error", .code=500};
    }

    if (service_select_stats_paginated(db, &request, stats) < 0){
        fprintf(stderr, "Failed to select stats from database\n");
        free(stats);
        sqlite3_close(db);
        return &(api_response_t){.reason="Internal Server Error", .code=500};
    }

    size_t body_size = page_size * STAT_SIZE;
    char* body = malloc(body_size);

    if (!body) {
        fprintf(stderr, "Memory allocation failed for response body\n");
        sqlite3_close(db);
        free(stats);
        return &(api_response_t){.reason="Internal Server Error", .code=500};
    }

    snprintf(body, body_size, "{ \"items\": [");

    for (int i = 0; i < request.count; i++) {
        char buf[STAT_SIZE];
        snprintf(buf, sizeof(buf),
                "{\"id\": %d, \"date\": \"%s\", \"time\": \"%s\", \"clock_amount\": %d, \"employee_id\": %d }%s",
                stats[i].id, stats[i].date, stats[i].time, stats[i].clock_amount, stats[i].employee_id,
                (i < page_size - 1 && i < request.count - 1) ? "," : "");
        strncat(body, buf, body_size - strlen(body) - 1);
    }

    strncat(body, "], ", body_size - strlen(body) - 1);
    char count_buf[64];
    snprintf(count_buf, sizeof(count_buf), "\"total_count\": %d", request.total_count);
    strncat(body, count_buf, body_size - strlen(body) - 1);

    strncat(body, "}", body_size - strlen(body) - 1);
    sqlite3_close(db);

    api_response_t* response = &(api_response_t){.body = body, .code = 200 , .reason="OK"};
    return response;
}

api_response_t* select_stats_for_employee(void* args){
    api_request_t* request_param = (api_request_t*)args;
    sqlite3* db = NULL;

    if(open_database(&db) < 0) {
        fprintf(stderr, "Failed to open database\n");
        return &(api_response_t){.reason="Internal Server Error", .code=500};
    }

    int page = 1;
    int page_size = PAGE_SIZE;

    const int employee_id = atoi(request_param->params);

    parse_paginated_query(request_param->query, &page, &page_size);
    paginated_request_t request = {.page=page, .page_size=page_size};

    Stat* stats = calloc(page_size, sizeof(Stat) * page_size);

    if (!stats) {
        fprintf(stderr, "Memory allocation failed for stats\n");
        sqlite3_close(db);
        return &(api_response_t){.reason="Internal Server Error", .code=500};
    }

    if (service_select_stats_by_employee(db, &request, stats, employee_id) < 0){
        fprintf(stderr, "Failed to select stats from database\n");
        free(stats);
        sqlite3_close(db);
        return &(api_response_t){.reason="Internal Server Error", .code=500};
    }

    size_t body_size = page_size * STAT_SIZE;
    char* body = malloc(body_size);

    if (!body) {
        fprintf(stderr, "Memory allocation failed for response body\n");
        sqlite3_close(db);
        free(stats);
        return &(api_response_t){.reason="Internal Server Error", .code=500};
    }

    snprintf(body, body_size, "{ \"items\": [");

    for (int i = 0; i < request.count; i++) {
        char buf[STAT_SIZE];
        snprintf(buf, sizeof(buf),
                "{\"id\": %d, \"date\": \"%s\", \"time\": \"%s\", \"clock_amount\": %d, \"employee_id\": %d }%s",
                stats[i].id, stats[i].date, stats[i].time, stats[i].clock_amount, stats[i].employee_id,
                (i < page_size - 1 && i < request.count - 1) ? "," : "");
        strncat(body, buf, body_size - strlen(body) - 1);
    }

    strncat(body, "], ", body_size - strlen(body) - 1);
    char count_buf[64];
    snprintf(count_buf, sizeof(count_buf), "\"total_count\": %d", request.total_count);
    strncat(body, count_buf, body_size - strlen(body) - 1);
    
    strncat(body, "}", body_size - strlen(body) - 1);
    sqlite3_close(db);

    api_response_t* response = &(api_response_t){.body = body, .code = 200 , .reason="OK"};
    return response;
}

api_response_t* add_stat(void* args){
    api_request_t* request_params = (api_request_t*)args;
    sqlite3* db = NULL;

    if (open_database(&db) < 0){
        fprintf(stderr, "Failed to open database\n");
        return &(api_response_t){.reason="Internal Server Error", .code=500};
    }

    json_remove_spaces(request_params->body);
    Stat* stat = parse_stat_json(request_params->body);

    if (service_add_stat(db, stat) < 0) {
        fprintf(stderr, "Failed to add stats\n");
        return &(api_response_t){.reason="Internal Server Error", .code=500};
    }

    api_response_t * response = &(api_response_t){.body = "Stat created", .code=201, .reason="Created"};
    return response;
}

api_response_t* delete_stat(void* args){
    api_request_t* request_params = (api_request_t*)args;
    sqlite3* db = NULL;

    if (open_database(&db) < 0) {
        fprintf(stderr, "Failed to open database\n");
        return &(api_response_t){.reason="Internal Server Error", .code=500};
    }

    const int id = atoi(request_params->params);

    if (service_delete_stat_by_id(db, id) < 0){
        fprintf(stderr, "Failed to delete employee from database\n");
        sqlite3_close(db);
        return &(api_response_t){.reason="Bad Request", .code=400};
    }

    sqlite3_close(db);

    api_response_t* response = &(api_response_t){.code = 204, .reason="No Content"};
    return response;
}

api_response_t* update_stat(void* args){
    api_request_t* request_params = (api_request_t*)args;
    sqlite3* db = NULL;

    if (open_database(&db) < 0){
        fprintf(stderr, "Failed to open database\n");
        return &(api_response_t){.reason="Internal Server Error",.code=500};
    }

    json_remove_spaces(request_params->body);
    Stat* stat = parse_stat_json(request_params->body);

    if(stat->id <= 0) {
        fprintf(stderr, "No id provided\n");
        return &(api_response_t){.reason="Bad Request", .code=400};
    }

    if(service_update_stat(db, stat) < 0) {
        fprintf(stderr, "Stat with provided ID doesn't exist\n");
        return &(api_response_t){.reason="Bad Request", .code=400};
    }

    api_response_t* response = &(api_response_t){.body = "Stat updated", .code=200, .reason="OK"};
    return response;
}

static Stat* parse_stat_json(char* json){
    Stat* s = calloc(1, sizeof(Stat));
    char *str1, *token;
    char *saveptr1;

    // Clearing braces
    json += 1;
    json[strlen(json) - 1] = '\0';

    for (str1 = json; ; str1 = NULL) {
        token = strtok_r(str1, ",", &saveptr1);
        if (!token) break;

        char *colon = strchr(token, ':');
        if (!colon) continue;

        *colon = '\0';
        char *subtoken = json_trim(token);
        char *value = json_trim(colon + 1);

        // remove quotes
        if (value[0] == '"') value++;
        int len = strlen(value);
        if (value[len - 1] == '"') value[len - 1] = '\0';

        if (strcmp("id", subtoken) == 0) s->id = atoi(value);
        else if (strcmp("date", subtoken) == 0) strcpy(s->date, value);
        else if (strcmp("time", subtoken) == 0) strcpy(s->time, value);
        else if (strcmp("clock_amount", subtoken) == 0) s->clock_amount = atoi(value);
        else if (strcmp("employee_id", subtoken) == 0) s->employee_id = atoi(value);
    }


    return s;
}

static void json_remove_spaces(char *s) {
    int in_quotes = 0, j = 0;
    for (int i = 0; s[i]; i++) {
        if (s[i] == '"') in_quotes = !in_quotes;
        if (!in_quotes && (s[i] == ' ' || s[i] == '\n' || s[i] == '\t'))
            continue;
        s[j++] = s[i];
    }
    s[j] = '\0';
}