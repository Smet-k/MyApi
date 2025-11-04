#include "config/db.h"

#include <stdio.h>

static int table_exists(sqlite3* db, const char* table_name);

int open_database(sqlite3** db) {
    int rc = sqlite3_open(DATABASE_NAME, db);

    char* errMsg = 0;

    if (rc) {
        fprintf(stderr, "Can't open database: %s\n", errMsg);
        return -1;
    }

    return 0;
}

int load_database() {
    sqlite3* db;
    int rc = sqlite3_open(DATABASE_NAME, &db);
    char* errMsg = 0;
    if (rc) {
        fprintf(stderr, "Can't open database: %s\n", errMsg);
        return -1;
    }

    if (!table_exists(db, "Roles")) {
        rc = sqlite3_exec(db,
                          "CREATE TABLE Roles ("
                          "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                          "name TEXT UNIQUE NOT NULL);",
                          0, 0, &errMsg);

        if (rc == SQLITE_OK)
            printf("Table 'Roles' created successfully\n");

        const char* sql_insert_positions = "INSERT INTO Roles (name) VALUES ('employee'), ('admin'), ('sysadmin')";

        sqlite3_exec(db, sql_insert_positions, 0, 0, &errMsg);
    }

    if (!table_exists(db, "Positions")) {
        rc = sqlite3_exec(db,
                          "CREATE TABLE Positions ("
                          "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                          "title TEXT NOT NULL,"
                          "salary INTEGER);",
                          0, 0, &errMsg);

        if (rc == SQLITE_OK)
            printf("Table 'Positions' created successfully\n");

        const char* sql_insert_positions =
            "INSERT INTO Positions (title, salary) VALUES ('Trainee', 200);"
            "INSERT INTO Positions (title, salary) VALUES ('Associate', 700);"
            "INSERT INTO Positions (title, salary) VALUES ('Junior', 1400);"
            "INSERT INTO Positions (title, salary) VALUES ('Middle', 3200);"
            "INSERT INTO Positions (title, salary) VALUES ('Senior', 5000);";

        sqlite3_exec(db, sql_insert_positions, 0, 0, &errMsg);
    }

    if (!table_exists(db, "Employees")) {
        rc = sqlite3_exec(db,
                          "CREATE TABLE Employees ("
                          "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                          "name TEXT NOT NULL,"
                          "surname TEXT NOT NULL,"
                          "position_id INTEGER,"
                          "role_id INTEGER,"
                          "FOREIGN KEY (position_id) REFERENCES Positions(id)"
                          " ON DELETE SET NULL ON UPDATE CASCADE,"
                          "FOREIGN KEY (role_id) REFERENCES Roles(id)"
                          ");",
                          0, 0, &errMsg);

        if (rc == SQLITE_OK)
            printf("Table 'Employees' created successfully\n");
        else
            fprintf(stderr, "Failed to create Employees: %s\n", errMsg);

        const char* sql_insert_employees =
            "INSERT INTO Employees (name, surname, position_id, role_id) VALUES ('John', 'Doe', 1, 1);"
            "INSERT INTO Employees (name, surname, position_id, role_id) VALUES ('Jane', 'Doe', 2, 1);"
            "INSERT INTO Employees (name, surname, position_id, role_id) VALUES ('Edmund', 'McMillen', 4, 1);";

        sqlite3_exec(db, sql_insert_employees, 0, 0, &errMsg);
    }

    if (!table_exists(db, "News")) {
        rc = sqlite3_exec(db,
                          "CREATE TABLE News ("
                          "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                          "title TEXT NOT NULL,"
                          "body TEXT NOT NULL,"
                          "date TEXT NOT NULL"
                          ");",
                          0, 0, &errMsg);

        if (rc == SQLITE_OK)
            printf("Table 'News' created successfully\n");
        else {
            fprintf(stderr, "Failed to create 'News': %s\n", errMsg);
            sqlite3_free(errMsg);
        }

        const char* sql_insert_news =
            "INSERT INTO News (title, body, date) VALUES "
            "('System Maintenance', 'The system will be down for maintenance tonight at 11 PM.', '2025-11-04'),"
            "('New Employee Portal', 'A new employee management portal has been launched.', '2025-10-28'),"
            "('Holiday Announcement', 'The office will be closed on December 25th and 26th.', '2025-10-20');";

        rc = sqlite3_exec(db, sql_insert_news, 0, 0, &errMsg);
    }

    sqlite3_close(db);
    return 0;
}

static int table_exists(sqlite3* db, const char* table_name) {
    sqlite3_stmt* stmt;
    const char* sql = "SELECT name FROM sqlite_master WHERE type='table' AND name=?;";
    int exists = 0;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, table_name, -1, SQLITE_STATIC);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            exists = 1; 
        }
    }
    sqlite3_finalize(stmt);
    return exists;
}

int get_entity_count(sqlite3* db, int* count, char* table) {
    char sql[256];
    sqlite3_stmt* stmt;

    snprintf(sql, sizeof(sql), "SELECT COUNT(*) FROM %s;", table);

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
