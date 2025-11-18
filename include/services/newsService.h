#ifndef NEWS_SERVICE_H
#define NEWS_SERVICE_H
#include "config/db.h"
#include "api_response.h"

typedef struct {
    int id;
    char title[50];
    char body[800];
    char date[16];
} Newsletter;

int service_select_news_paginated(sqlite3* db, paginated_request_t* request, Newsletter *out);
int service_select_news_by_id(sqlite3* db, int id, Newsletter* out);
int service_add_news(sqlite3* db, const Newsletter* n);
int service_update_news(sqlite3* db, const Newsletter* n);
int service_delete_news_by_id(sqlite3* db, int id);

#endif