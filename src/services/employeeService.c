#include "services/employeeService.h"

#include <stdio.h>
#include <stdlib.h>

static int get_employee_count(sqlite3 *db, int *count);

int service_select_employees_paginated(sqlite3* db, int page, int page_size, Employee* out, int* count) {
    const char* sql = "SELECT id, name, surname, position_id FROM Employees LIMIT ? OFFSET ?;";
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
        snprintf(out[idx].name, sizeof(out[idx].name), "%s", sqlite3_column_text(stmt, 1));
        snprintf(out[idx].surname, sizeof(out[idx].surname), "%s", sqlite3_column_text(stmt, 2));
        out[idx].position_id = sqlite3_column_int(stmt, 3);
        idx++;
    }

    if(get_employee_count(db, count) < 0){
        fprintf(stderr, "Failed to get employee count\n");
        sqlite3_finalize(stmt);
        return -1;
    }
    sqlite3_finalize(stmt);
    return 0;
}

int service_select_employee_by_id(sqlite3* db, int id, Employee* out) {
    const char* sql = "SELECT id, name, surname, position_id FROM Employees WHERE id = ?;";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Prepare failed: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    sqlite3_bind_int(stmt, 1, id);

    int rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        out->id = sqlite3_column_int(stmt, 0);
        snprintf(out->name, sizeof(out->name), "%s", sqlite3_column_text(stmt, 1));
        snprintf(out->surname, sizeof(out->name), "%s", sqlite3_column_text(stmt, 2));
        out->position_id = sqlite3_column_int(stmt, 3);
    } else if (rc == SQLITE_DONE) {
        fprintf(stderr, "No employee found with id=%d\n", id);
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

int service_add_employee(sqlite3* db, const Employee* e) {
    const char* sql = "INSERT INTO Employees (name, surname, position_id) VALUES (?, ?, ?);";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Prepare failed: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    sqlite3_bind_text(stmt, 1, e->name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, e->surname, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, e->position_id);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        fprintf(stderr, "Insert failed: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    return 0;
}

int service_update_employee(sqlite3* db, const Employee* e) {
    const char* sql = "UPDATE Employees SET name = ?, surname = ?, position_id = ? WHERE id = ?;";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Prepare failed: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    sqlite3_bind_text(stmt, 1, e->name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, e->surname, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, e->position_id);
    sqlite3_bind_int(stmt, 4, e->id);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        fprintf(stderr, "Update failed: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return -1;
    }

    int changes = sqlite3_changes(db);
    sqlite3_finalize(stmt);

    if (changes == 0) {
        fprintf(stderr, "No employee with ID %d found.\n", e->id);
        return -1;
    }

    sqlite3_finalize(stmt);
    return 0;
}

int service_delete_employee_by_id(sqlite3* db, int id) {
    const char* sql = "DELETE FROM Employees WHERE id = ?;";
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

static int get_employee_count(sqlite3 *db, int *count) {
    const char *sql = "SELECT COUNT(*) FROM Employees;";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Prepare failed: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        *count = sqlite3_column_int(stmt, 0);
    } else {
        fprintf(stderr, "Failed to count rows: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    return 0;
}
