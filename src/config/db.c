#include "config/db.h"

#include <stdio.h>

static int table_exists(sqlite3* db, const char* table_name);

int open_database(sqlite3** db) {
    int rc = sqlite3_open(DATABASE_NAME, db);

    char* errMsg = 0;

    if(rc){
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

    if (!table_exists(db, "Positions")) {
        rc = sqlite3_exec(db,
                          "CREATE TABLE Positions ("
                          "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                          "title TEXT NOT NULL,"
                          "salary REAL);",
                          0, 0, &errMsg);

        if (rc == SQLITE_OK)
            printf("Table 'Positions' created successfully\n");

        const char* sql_insert_positions =
            "INSERT INTO Positions (title, salary) VALUES ('Trainee', 200.0);"
            "INSERT INTO Positions (title, salary) VALUES ('Associate', 700.0);"
            "INSERT INTO Positions (title, salary) VALUES ('Junior', 1400.0);"
            "INSERT INTO Positions (title, salary) VALUES ('Middle', 3200.0);"
            "INSERT INTO Positions (title, salary) VALUES ('Senior', 5000.0);";

        sqlite3_exec(db, sql_insert_positions, 0, 0, &errMsg);
    }

    if (!table_exists(db, "Employees")) {
        rc = sqlite3_exec(db,
                          "CREATE TABLE Employees ("
                          "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                          "name TEXT NOT NULL,"
                          "surname TEXT NOT NULL,"
                          "position_id INTEGER,"
                          "FOREIGN KEY (position_id) REFERENCES Positions(id)"
                          " ON DELETE SET NULL ON UPDATE CASCADE);",
                          0, 0, &errMsg);

        if (rc == SQLITE_OK)
            printf("Table 'Employees' created successfully\n");

        const char* sql_insert_employees =
            "INSERT INTO Employees (name, surname, position_id) VALUES ('John', 'Doe', 1);"
            "INSERT INTO Employees (name, surname, position_id) VALUES ('Jane', 'Doe', 2);"
            "INSERT INTO Employees (name, surname, position_id) VALUES ('Edmund', 'McMillen', 4);";

        sqlite3_exec(db, sql_insert_employees, 0, 0, &errMsg);
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
            exists = 1;  // table found
        }
    }
    sqlite3_finalize(stmt);
    return exists;
}