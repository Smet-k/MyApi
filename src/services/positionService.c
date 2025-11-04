#include "services/positionService.h"

#include <stdio.h>
#include <stdlib.h>

int service_select_positions_paginated(sqlite3* db, int page, int page_size, Position* out, int* count){
    const char* sql = "SELECT id, title, salary FROM Positions LIMIT ? OFFSET ?;";
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
        out[idx].salary = sqlite3_column_int(stmt, 2);
        idx++;
    } 

    if(get_entity_count(db, count, "Positions") < 0){
        fprintf(stderr, "Failed to get position count\n");
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    return 0;
}

int service_select_position_by_id(sqlite3* db, int id, Position* out){
    const char* sql = "SELECT id, title, salary FROM Positions WHERE id = ?;";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Prepare failed: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    sqlite3_bind_int(stmt, 1, id);

    int rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW){
        out->id = sqlite3_column_int(stmt, 0);
        snprintf(out->title, sizeof(out->title), "%s", sqlite3_column_text(stmt, 1));
        out->salary = sqlite3_column_int(stmt, 2);
    } else if (rc == SQLITE_DONE){
        fprintf(stderr, "No position found with id=%d\n", id);
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

int service_add_position(sqlite3* db, const Position* p){
    const char* sql = "INSERT INTO Positions (title, salary) VALUES (?, ?);";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Prepare failed: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    sqlite3_bind_text(stmt, 1, p->title, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, p->salary);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        fprintf(stderr, "Insert failed: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return -1;
    } 

    sqlite3_finalize(stmt);
    return 0;
}
int service_update_position(sqlite3* db, const Position* p){
    const char* sql = "UPDATE Positions SET title = ?, salary = ? WHERE id = ?;";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Prepare failed: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    sqlite3_bind_text(stmt, 1, p->title, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, p->salary);
    sqlite3_bind_int(stmt, 3, p->id);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        fprintf(stderr, "Update failed: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return -1;
    }

    int changes = sqlite3_changes(db);

    if (changes == 0) {
        fprintf(stderr, "No position with ID %d found.\n", p->id);
        return -1;
    }

    sqlite3_finalize(stmt);
    return 0;
}

int service_delete_position_by_id(sqlite3* db, int id){
    const char* sql = "DELETE FROM Positions WHERE id = ?;";
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

