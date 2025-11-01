#ifndef EMPLOYEE_SERVICE_H
#define EMPLOYEE_SERVICE_H
#include "config/db.h"

typedef struct {
    int id;
    char name[50];
    char surname[50];
    int position_id;
} Employee;

int select_employees_paginated(sqlite3* db, int page, int page_size, Employee *out, int *count);
int select_employee_by_id(sqlite3* db, int id, Employee* out);
int add_employee_by_id(sqlite3* db, const Employee* e);
int update_employee_by_object(sqlite3* db, const Employee* e);
int delete_employee_by_id(sqlite3* db, int id);
#endif