#include "controllers/employeeController.h"
#include "services/employeeService.h"
#include "config/db.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#define PAGE_SIZE 10
#define EMPLOYEE_SIZE 256

static void parse_paginated_query(char* query, int* page, int* page_size);

api_response_t* select_employee(void* args){
    api_request_t* request_params = (api_request_t*)args;
    sqlite3* db = NULL;
    if(open_database(&db) < 0){
        fprintf(stderr, "Failed to open database\n");
        return NULL;
    }
    Employee employee;

    const int id = atoi(request_params->params);
    
    if(select_employee_by_id(db, id, &employee) < 0){
        fprintf(stderr, "Failed to select employee from database\n");
        sqlite3_close(db);
        return NULL;
    }

    char body[256];
    snprintf(body, sizeof(body),
        "{\"id\": %d, \"name\": \"%s\", \"surname\": \"%s\", \"position_id\": %d}",
        employee.id, employee.name, employee.surname, employee.position_id);
    
    sqlite3_close(db);

    api_response_t* response = &(api_response_t){.body=body, .code=200};
    return response;
}

api_response_t* select_employees(void* args){
    api_request_t* request_params = (api_request_t*)args;
    sqlite3* db = NULL;
    if(open_database(&db) < 0){
        fprintf(stderr, "Failed to open database\n");
        return NULL;
    }

    int count = 0;
    int page = 1;
    int page_size = PAGE_SIZE;

    parse_paginated_query(request_params->query, &page, &page_size);

    Employee* employees = malloc(sizeof(Employee) * page_size);

    if (!employees) {
        fprintf(stderr, "Memory allocation failed for employees\n");
        sqlite3_close(db);
        return NULL;
    }

    if(select_employees_paginated(db,page,page_size, employees, &count) < 0){
        fprintf(stderr, "Failed to select employees from database\n");
        free(employees);
        sqlite3_close(db);
        return NULL;
    }

    size_t body_size = page_size * EMPLOYEE_SIZE;
    char* body = malloc(body_size);
    if (!body) {
        fprintf(stderr, "Memory allocation failed for response body\n");
        sqlite3_close(db);
        free(employees);
        return NULL;
    }

    snprintf(body, body_size, "[");

    for(int i = 0; i < page_size && i < count; i++){
        char buf[EMPLOYEE_SIZE];
        snprintf(buf, sizeof(buf),
            "{\"id\": %d, \"name\": \"%s\", \"surname\": \"%s\", \"position_id\": %d}%s",
            employees[i].id, employees[i].name, employees[i].surname, employees[i].position_id,
            (i < page_size - 1 && i < count - 1) ? "," : "");
        strncat(body, buf, body_size - strlen(body) - 1);
    }

    strncat(body, "]" ,body_size - strlen(body) - 1);
    sqlite3_close(db);

    api_response_t* response = &(api_response_t){.body=body, .code=200};
    return response;
}

api_response_t* add_employee(void* args);
api_response_t* delete_employee(void* args);
api_response_t* update_employee(void* args);

static void parse_paginated_query(char* query, int* page, int* page_size){
    char *saveptr1, *saveptr2;
    char *pair = strtok_r(query, "&", &saveptr1);
    while (pair != NULL) {
        char *key = strtok_r(pair, "=", &saveptr2);
        char *value = strtok_r(NULL, "=", &saveptr2);

        if (key && value) {
            if (strcasecmp(key, "page") == 0) {
                *page = atoi(value);
            } else if (strcasecmp(key, "page_size") == 0) {
                *page_size = atoi(value);
            }
        }

        pair = strtok_r(NULL, "&", &saveptr1);
    }
}