#ifndef POSITION_SERVICE_H
#define POSITION_SERVICE_H
#include "config/db.h"
#include "api_response.h"

typedef struct {
    int id;
    char title[50];
    int salary;
} Position;

int service_select_positions_paginated(sqlite3* db, paginated_request_t* request, Position* out);
int service_select_position_by_id(sqlite3* db, int id, Position* out);
int service_add_position(sqlite3* db, const Position* p);
int service_update_position(sqlite3* db, const Position* p);
int service_delete_position_by_id(sqlite3* db, int id);
#endif