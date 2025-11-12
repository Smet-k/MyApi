#ifndef POSITION_SERVICE_H
#define POSITION_SERVICE_H
#include "config/db.h"

typedef struct {
    int id;
    char date[16];
    char time[24];
    int clock_amount;
    int employee_id;
} Stat;

int service_select_stats_paginated(sqlite3* db, int page, int page_size, Stat* out, int* count);
int service_select_stat_by_id(sqlite3* db, int id, Stat* out);
int service_add_stat(sqlite3* db, const Stat* s);
int service_update_stat(sqlite3* db, const Stat* s);
int service_delete_stat_by_id(sqlite3* db, int id);
#endif