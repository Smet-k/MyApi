#include "services/newsService.h"

#include <stdio.h>
#include <stdlib.h>

int service_select_news_paginated(sqlite3* db, int page, int page_size, Newsletter *out, int *count){
    const char* sql = "SELECT id, title, body, date FROM News LIMIT ? OFFSET ?;";
    sqlite3_stmt* stmt;
    int offset = (page - 1) * page_size;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK){
        fprintf(stderr, "Prepare failed: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    sqlite3_bind_int(stmt, 1, page_size);
    sqlite3_bind_int(stmt, 2, offset);

    int idx = 0;
    while(sqlite3_step(stmt) == SQLITE_ROW && idx < page_size){
        out[idx].id = sqlite3_column_int(stmt, 0);
        snprintf(out[idx].title, sizeof(out[idx].title), "%s", sqlite3_column_text(stmt, 1));
        snprintf(out[idx].body, sizeof(out[idx].body), "%s", sqlite3_column_text(stmt, 2));
        snprintf(out[idx].date, sizeof(out[idx].date), "%s", sqlite3_column_text(stmt, 3));
        idx++;
    }

    if(get_entity_count(db, count, "News") < 0){
        fprintf(stderr, "Failed to get news count\n");
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    return 0;
}

int service_select_news_by_id(sqlite3* db, int id, Newsletter* out){
    const char* sql = "SELECT id, title, body, date, FROM News WHERE id = ?;";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Prepare failed: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    sqlite3_bind_int(stmt, 1, id);

    int rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        out->id = sqlite3_column_int(stmt, 0);
        snprintf(out->title, sizeof(out->title), "%s", sqlite3_column_text(stmt, 1));
        snprintf(out->body, sizeof(out->body), "%s", sqlite3_column_text(stmt, 2));
        snprintf(out->date, sizeof(out->date), "%s", sqlite3_column_text(stmt, 3));
    } else if (rc == SQLITE_DONE) {
        fprintf(stderr, "No news found with id=%d\n", id);
        sqlite3_finalize(stmt);
        return -1;
    } else{
        fprintf(stderr, "Select failed: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    return 0;
}

int service_add_news(sqlite3* db, const Newsletter* n){
    const char* sql = "INSERT INTO News (title, body, date) VALUES (?, ?, ?);";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Prepare failed: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    sqlite3_bind_text(stmt, 1, n->title, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, n->body, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, n->date, -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        fprintf(stderr, "Insert failed: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    return 0;
}

int service_update_news(sqlite3* db, const Newsletter* n){
    const char* sql = "UPDATE News SET title = ?, body = ?, date = ?, WHERE id = ?;";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Prepare failed: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    sqlite3_bind_text(stmt, 1, n->title, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, n->body, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, n->date, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 4, n->id);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        fprintf(stderr, "Update failed: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return -1;
    }

    int changes = sqlite3_changes(db);

    if (changes == 0) {
        fprintf(stderr, "No news with ID %d found.\n", n->id);
        return -1;
    }

    sqlite3_finalize(stmt);
    return 0;
}

int service_delete_news_by_id(sqlite3* db, int id){
    const char* sql = "DELETE FROM News WHERE id = ?;";
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
