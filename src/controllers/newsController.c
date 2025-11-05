#include "controllers/newsController.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "config/db.h"
#include "services/newsService.h"

#define PAGE_SIZE 10
#define NEWS_SIZE 1024

static Newsletter* parse_news_json(char* json);

api_response_t* select_newsletter(void* args){
    api_request_t* request_params = (api_request_t*)args;
    sqlite3* db = NULL;

    if (open_database(&db) < 0) {
        fprintf(stderr, "Failed to open database\n");
        return &(api_response_t){.body="Internal Server Error", .code=500};
    }
    Newsletter news;

    const int id = atoi(request_params->params);

    if(service_select_news_by_id(db, id, &news) < 0){
        fprintf(stderr, "Failed to select news from database\n");
        sqlite3_close(db);
        return &(api_response_t){.body="Bad Request", .code=400};
    }

    char body[NEWS_SIZE + 64];
    snprintf(body, sizeof(body),
             "{\"id\": %d, \"title\": \"%s\", \"body\": \"%s\", \"date\": %s}",
             news.id, news.title, news.body, news.date);

    sqlite3_close(db);

    api_response_t* response = &(api_response_t){.body = body, .code = 200};
    return response;
}

api_response_t* select_news(void* args){
    api_request_t* request_params = (api_request_t*)args;
    sqlite3* db = NULL;
    if (open_database(&db) < 0) {
        fprintf(stderr, "Failed to open database\n");
        return &(api_response_t){.body="Internal Server Error", .code=500};
    }

    int count = 0;
    int page = 1;
    int page_size = PAGE_SIZE;

    parse_paginated_query(request_params->query, &page, &page_size);

    Newsletter* news = calloc(page_size, sizeof(Newsletter) * page_size);

    if(service_select_news_paginated(db, page, page_size, news, &count) < 0){
        fprintf(stderr, "Failed to select news from database\n");
        free(news);
        sqlite3_close(db);
        return &(api_response_t){.body="Internal Server Error", .code=500};
    }

    size_t body_size = page_size * NEWS_SIZE;
    char* body = malloc(body_size);

    if (!body) {
        fprintf(stderr, "Memory allocation failed for response body\n");
        sqlite3_close(db);
        free(news);
        return &(api_response_t){.body="Internal Server Error", .code=500};
    }

    snprintf(body, body_size, "[");

    for (int i = 0; i < page_size && i < count; i++) {
        char buf[NEWS_SIZE];
        snprintf(buf, sizeof(buf),
                 "{\"id\": %d, \"title\": \"%s\", \"body\": \"%s\", \"date\": %s}%s",
                 news[i].id, news[i].title, news[i].body, news[i].date,
                 (i < page_size - 1 && i < count - 1) ? "," : "");
        strncat(body, buf, body_size - strlen(body) - 1);
    }

    strncat(body, "]", body_size - strlen(body) - 1);
    sqlite3_close(db);

    api_response_t* response = &(api_response_t){.body = body, .code = 200};
    return response;
}

api_response_t* add_news(void* args){
    api_request_t* request_params = (api_request_t*)args;
    sqlite3* db = NULL;

    if (open_database(&db) < 0) {
        fprintf(stderr, "Failed to open database\n");
        return &(api_response_t){.body="Internal Server Error", .code=500};
    }

    Newsletter* news = parse_news_json(request_params->body);

    if (service_add_news(db, news) < 0){
        fprintf(stderr, "Failed to add news\n");
        return &(api_response_t){.body="Internal Server Error", .code=500};
    }

    api_response_t* response = &(api_response_t){.body = "News created", .code = 201};
    return response;
}

api_response_t* delete_news(void* args){
    api_request_t* request_params = (api_request_t*)args;
    sqlite3* db = NULL;

    if (open_database(&db) < 0){
        fprintf(stderr, "Failed to open database\n");
        return &(api_response_t){.body="Internal Server Error", .code=500};
    }

    const int id = atoi(request_params->params);

    if (service_delete_news_by_id(db, id) < 0){
        fprintf(stderr, "Failed to delete news from database\n");
        sqlite3_close(db);
        return &(api_response_t){.body="Bad Request", .code=400};
    }

    sqlite3_close(db);

    api_response_t* response = &(api_response_t){.body = "News deleted", .code=204};
    return response;
}

api_response_t* update_news(void* args){
    api_request_t* request_params = (api_request_t*)args;
    sqlite3* db = NULL;

    if (open_database(&db) < 0){
        fprintf(stderr, "Failed to open database\n");
        return &(api_response_t){.body="Internal Server Error", .code=500};
    }

    Newsletter* news = parse_news_json(request_params->body);

    if (news->id <= 0) {
        fprintf(stderr, "No id provided\n");
        return &(api_response_t){.body="Bad Request", .code=400};
    }

    if(service_update_news(db, news) < 0) {
        fprintf(stderr, "News with provided Id doesn't exist\n");
        return &(api_response_t){.body="Bad Request", .code=400};
    }

    api_response_t* response = &(api_response_t){.body = "News updated.", .code = 200};
    return response;
}

static Newsletter* parse_news_json(char* json) {
    Newsletter* n = calloc(1, sizeof(Newsletter));
    char *str1, *token, *subtoken;
    char *saveptr1, *saveptr2;

    // Clearing braces
    json += 1;
    json[strlen(json) - 1] = '\0';

   for (str1 = json; ; str1 = NULL) {
        token = strtok_r(str1, ",", &saveptr1);
        if (token == NULL)
            break;

        subtoken = json_trim(strtok_r(token, ":", &saveptr2));
        char* value =  json_trim(strtok_r(NULL, ":", &saveptr2));
        
        if (strcmp("id", subtoken) == 0) n->id = atoi(value);
        else if(strcmp("title", subtoken) == 0) strcpy(n->title, value);
        else if(strcmp("body", subtoken) == 0) strcpy(n->body, value);
        else if(strcmp("date", subtoken) == 0) strcpy(n->date, value);
    }

    return n;
}

