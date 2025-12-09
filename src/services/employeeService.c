#include "services/employeeService.h"

#include <stdio.h>
#include <stdlib.h>


int service_select_employees_paginated(sqlite3* db, paginated_request_t* request, char* search, Employee* out) {
    const char* sql =
    "SELECT id, login, name, surname, employment_date, position_id, role_id, password FROM Employees "
    "WHERE (? IS NULL OR ? = '' OR name LIKE '%' || ? || '%' "
    " OR surname LIKE '%' || ? || '%') LIMIT ? OFFSET ?;";

    sqlite3_stmt* stmt;
    int offset = (request->page - 1) * request->page_size;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK){
        fprintf(stderr, "Prepare failed: %s\n", sqlite3_errmsg(db));
        return -1;
    }


    sqlite3_bind_text(stmt, 1, search, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, search, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, search, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, search, -1, SQLITE_STATIC);

    sqlite3_bind_int(stmt, 5, request->page_size);
    sqlite3_bind_int(stmt, 6, offset);


    int idx = 0;
    while(sqlite3_step(stmt) == SQLITE_ROW && idx < request->page_size){
        out[idx].id = sqlite3_column_int(stmt, 0);
        snprintf(out[idx].login, sizeof(out[idx].login), "%s", sqlite3_column_text(stmt, 1));
        snprintf(out[idx].name, sizeof(out[idx].name), "%s", sqlite3_column_text(stmt, 2));
        snprintf(out[idx].surname, sizeof(out[idx].surname), "%s", sqlite3_column_text(stmt, 3));
        snprintf(out[idx].date, sizeof(out[idx].date), "%s", sqlite3_column_text(stmt, 4));
        out[idx].position_id = sqlite3_column_int(stmt, 5);
        out[idx].role = sqlite3_column_int(stmt, 6);
        snprintf(out[idx].password, sizeof(out[idx].password), "%s", sqlite3_column_text(stmt, 7));
        idx++;
    }

    if(get_entity_count(db, &request->total_count, "Employees") < 0){
        fprintf(stderr, "Failed to get employee count\n");
        sqlite3_finalize(stmt);
        return -1;
    }

    request->count = idx;

    sqlite3_finalize(stmt);
    return 0;
}

int service_select_employee_by_id(sqlite3* db, int id, Employee* out) {
    const char* sql = "SELECT id, login, name, surname, employment_date, position_id, role_id, password FROM Employees WHERE id = ?;";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Prepare failed: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    sqlite3_bind_int(stmt, 1, id);

    int rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        out->id = sqlite3_column_int(stmt, 0);
        snprintf(out->login, sizeof(out->login), "%s", sqlite3_column_text(stmt, 1));
        snprintf(out->name, sizeof(out->name), "%s", sqlite3_column_text(stmt, 2));
        snprintf(out->surname, sizeof(out->name), "%s", sqlite3_column_text(stmt, 3));
        snprintf(out->date, sizeof(out->date), "%s", sqlite3_column_text(stmt, 4));
        out->position_id = sqlite3_column_int(stmt, 5);
        out->role = sqlite3_column_int(stmt, 6);
        snprintf(out->password, sizeof(out->password), "%s", sqlite3_column_text(stmt, 7));
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

int service_select_employee_by_login(sqlite3* db, char* login, Employee* out){
    const char* sql = "SELECT id, login, name, surname, employment_date, position_id, role_id, password FROM Employees WHERE login = ? COLLATE BINARY;";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Prepare failed: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    sqlite3_bind_text(stmt, 1, login, -1,SQLITE_STATIC);

    int rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        out->id = sqlite3_column_int(stmt, 0);
        snprintf(out->login, sizeof(out->login), "%s", sqlite3_column_text(stmt, 1));
        snprintf(out->name, sizeof(out->name), "%s", sqlite3_column_text(stmt, 2));
        snprintf(out->surname, sizeof(out->surname), "%s", sqlite3_column_text(stmt, 3));
        snprintf(out->date, sizeof(out->date), "%s", sqlite3_column_text(stmt, 4));
        out->position_id = sqlite3_column_int(stmt, 5);
        out->role = sqlite3_column_int(stmt, 6);
        snprintf(out->password, sizeof(out->password), "%s", sqlite3_column_text(stmt, 7));
    } else if (rc == SQLITE_DONE) {
        fprintf(stderr, "No employee found with name=%s\n", login);
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
    const char* sql = "INSERT INTO Employees (login, name, surname, employment_date, position_id, role_id, password) VALUES (?, ?, ?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Prepare failed: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    sqlite3_bind_text(stmt, 1, e->login, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, e->name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, e->surname, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, e->date, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 5, e->position_id);
    sqlite3_bind_int(stmt, 6, e->role);
    sqlite3_bind_text(stmt, 7, e->password, -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        fprintf(stderr, "Insert failed: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    return 0;
}

int service_update_employee(sqlite3* db, const Employee* e) {
    const char* sql = "UPDATE Employees SET login = ?, name = ?, surname = ?, employment_date = ?, position_id = ?, role_id = ?, password = ? WHERE id = ?;";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Prepare failed: %s\n", sqlite3_errmsg(db));
        return -1;
    }
    sqlite3_bind_text(stmt, 1, e->login, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, e->name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, e->surname, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, e->date, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 5, e->position_id);
    sqlite3_bind_int(stmt, 6, e->role);
    sqlite3_bind_text(stmt, 7, e->password, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 8, e->id);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        fprintf(stderr, "Update failed: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return -1;
    }

    int changes = sqlite3_changes(db);

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
