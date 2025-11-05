#ifndef EMPLOYEE_SERVICE_H
#define EMPLOYEE_SERVICE_H
#include "config/db.h"

typedef enum {
    ROLE_EMPLOYEE = 1,
    ROLE_ADMIN,
    ROLE_SYSADMIN
} RoleType;

typedef struct {
    int id;
    char name[50];
    char surname[50];
    int position_id;
    RoleType role;
    char password[80];
} Employee;

int service_select_employees_paginated(sqlite3* db, int page, int page_size, Employee *out, int *count);
int service_select_employee_by_id(sqlite3* db, int id, Employee* out);
int service_add_employee(sqlite3* db, const Employee* e);
int service_update_employee(sqlite3* db, const Employee* e);
int service_delete_employee_by_id(sqlite3* db, int id);
#endif