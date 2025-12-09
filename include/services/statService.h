#ifndef POSITION_SERVICE_H
#define POSITION_SERVICE_H
#include "config/db.h"
#include "api_response.h"

typedef struct {
    int id;
    char date[16];
    char time[24];
    int clock_amount;
    int employee_id;
} Stat;

int service_select_stats_paginated(sqlite3* db, paginated_request_t* request, Stat* out);
int service_select_stat_by_id(sqlite3* db, int id, Stat* out);
int service_add_stat(sqlite3* db, const Stat* s);
int service_update_stat(sqlite3* db, const Stat* s);
int service_delete_stat_by_id(sqlite3* db, int id);
int service_select_stats_by_employee(sqlite3* db, paginated_request_t* request, Stat* out, int employee_id);
#endif