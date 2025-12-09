#include "services/statService.h"

#include <stdio.h>
#include <stdlib.h>

static int get_filtered_stat_count(sqlite3* db, int employee_id, int* out_count);

int service_select_stats_paginated(sqlite3* db, paginated_request_t* request, Stat* out){
    const char* sql = "SELECT id, date, time, clock_amount, employee_id FROM Stats LIMIT ? OFFSET ?;";
    sqlite3_stmt* stmt;
    int offset = (request->page - 1) * request->page_size;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK){
        fprintf(stderr, "Prepare failed: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    sqlite3_bind_int(stmt, 1, request->page_size);
    sqlite3_bind_int(stmt, 2, offset);

    int idx = 0;
    while(sqlite3_step(stmt) == SQLITE_ROW && idx < request->page_size){
        out[idx].id = sqlite3_column_int(stmt, 0);
        snprintf(out[idx].date, sizeof(out[idx].date), "%s", sqlite3_column_text(stmt, 1));
        snprintf(out[idx].time, sizeof(out[idx].time), "%s", sqlite3_column_text(stmt, 2));
        out[idx].clock_amount = sqlite3_column_int(stmt, 3);
        out[idx].employee_id = sqlite3_column_int(stmt, 4);
        idx++;
    }

    if (get_entity_count(db, &request->total_count, "Stats") < 0){
        fprintf(stderr, "Failed to get stat count\n");
        sqlite3_finalize(stmt);
        return -1;
    }

    request->count = idx;

    sqlite3_finalize(stmt);
    return 0;
}

int service_select_stats_by_employee(sqlite3* db, paginated_request_t* request, Stat* out, int employee_id){
    const char* sql = "SELECT id, date, time, clock_amount, employee_id FROM Stats WHERE employee_id = ? ORDER BY date DESC, time DESC LIMIT ? OFFSET ?;";
    sqlite3_stmt* stmt;
    int offset = (request->page - 1) * request->page_size;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK){
        fprintf(stderr, "Prepare failed: %s\n", sqlite3_errmsg(db));
        return -1;
    }
    
    sqlite3_bind_int(stmt, 1, employee_id);
    sqlite3_bind_int(stmt, 2, request->page_size);
    sqlite3_bind_int(stmt, 3, offset);

    int idx = 0;
    while(sqlite3_step(stmt) == SQLITE_ROW && idx < request->page_size){
        out[idx].id = sqlite3_column_int(stmt, 0);
        snprintf(out[idx].date, sizeof(out[idx].date), "%s", sqlite3_column_text(stmt, 1));
        snprintf(out[idx].time, sizeof(out[idx].time), "%s", sqlite3_column_text(stmt, 2));
        out[idx].clock_amount = sqlite3_column_int(stmt, 3);
        out[idx].employee_id = sqlite3_column_int(stmt, 4);
        idx++;
    }

    if (get_filtered_stat_count(db, employee_id, &request->total_count) < 0) {
        fprintf(stderr, "Failed to get stat count\n");
        sqlite3_finalize(stmt);
        return -1;
    }

    request->count = idx;

    sqlite3_finalize(stmt);
    return 0;
}


int service_select_stat_by_id(sqlite3* db, int id, Stat* out){
    const char* sql = "SELECT id, date, time, clock_amount, employee_id FROM Stats WHERE id = ?;";
    sqlite3_stmt* stmt;

    if(sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK){
        fprintf(stderr, "Prepare failed: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    sqlite3_bind_int(stmt, 1, id);

    int rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        out->id = sqlite3_column_int(stmt, 0);
        snprintf(out->date, sizeof(out->date), "%s", sqlite3_column_text(stmt, 1));
        snprintf(out->time, sizeof(out->time), "%s", sqlite3_column_text(stmt, 2));
        out->clock_amount = sqlite3_column_int(stmt, 3);
        out->employee_id = sqlite3_column_int(stmt, 4);
    } else if (rc == SQLITE_DONE) {
        fprintf(stderr, "No stat found with id=%d\n", id);
        sqlite3_finalize(stmt);
        return -1;
    } else {
        fprintf(stderr, "Select failed: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    return 0;
}

int service_add_stat(sqlite3* db, const Stat* s){
    const char* sql = "INSERT INTO Stats (date, time, clock_amount, employee_id) VALUES (?, ?, ?, ?);";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK){
        fprintf(stderr, "Prepare failed: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    sqlite3_bind_text(stmt, 1, s->date, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, s->time, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, s->clock_amount);
    sqlite3_bind_int(stmt, 4, s->employee_id);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        fprintf(stderr, "Insert failed: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    return 0;
}

int service_update_stat(sqlite3* db, const Stat* s){
    const char* sql = "UPDATE Stats SET date = ?, time = ?, clock_amount = ?, employee_id = ? WHERE id = ?;";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Prepare failed: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    sqlite3_bind_text(stmt, 1, s->date, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, s->time, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, s->clock_amount);
    sqlite3_bind_int(stmt, 4, s->employee_id);
    sqlite3_bind_int(stmt, 5, s->id);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        fprintf(stderr, "Update failed: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return -1;
    }

    int changes = sqlite3_changes(db);

    if (changes == 0) {
        fprintf(stderr, "No stats with ID %d found.\n", s->id);
        return -1;
    }

    sqlite3_finalize(stmt);
    return 0;
}

int service_delete_stat_by_id(sqlite3* db, int id){
    const char* sql = "DELETE FROM Stats WHERE id = ?;";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Prepare failed: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    sqlite3_bind_int(stmt, 1, id);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        fprintf(stderr, "Delete failed: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    return 0;
}

static int get_filtered_stat_count(sqlite3* db, int employee_id, int* out_count) {
    const char* sql = "SELECT COUNT(*) FROM Stats WHERE employee_id = ?;";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Prepare failed: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    sqlite3_bind_int(stmt, 1, employee_id);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        *out_count = sqlite3_column_int(stmt, 0);
    } else {
        *out_count = 0;
    }

    sqlite3_finalize(stmt);
    return 0;
}