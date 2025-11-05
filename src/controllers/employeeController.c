#include "controllers/employeeController.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "config/db.h"
#include "services/employeeService.h"

#define PAGE_SIZE 10
#define EMPLOYEE_SIZE 512

static Employee* parse_employee_json(char* json);

api_response_t* select_employee(void* args) {
    api_request_t* request_params = (api_request_t*)args;
    sqlite3* db = NULL;
    if (open_database(&db) < 0) {
        fprintf(stderr, "Failed to open database\n");
        return &(api_response_t){.body="Internal Server Error", .code=500};
    }
    Employee employee;

    const int id = atoi(request_params->params);

    if (service_select_employee_by_id(db, id, &employee) < 0) {
        fprintf(stderr, "Failed to select employee from database\n");
        sqlite3_close(db);
        return &(api_response_t){.body="Bad Request", .code=400};
    }

    char body[EMPLOYEE_SIZE + 64];
    snprintf(body, sizeof(body),
             "{\"id\": %d, \"name\": \"%s\", \"surname\": \"%s\", \"position_id\": %d, \"role_id\": %d, \"password\": \"%s\"}",
             employee.id, employee.name, employee.surname, employee.position_id, employee.role, employee.password);

    sqlite3_close(db);

    api_response_t* response = &(api_response_t){.body = body, .code = 200};
    return response;
}

api_response_t* select_employees(void* args) {
    api_request_t* request_params = (api_request_t*)args;
    sqlite3* db = NULL;
    if (open_database(&db) < 0) {
        fprintf(stderr, "Failed to open database\n");
        return &(api_response_t){.body="Internal Server Error", .code=500};
    }

    int count = 0;
    int page = 1;
    int page_size = PAGE_SIZE;

    parse_paginated_query(request_params->query, &page, &page_size);

    Employee* employees = calloc(page_size, sizeof(Employee) * page_size);

    if (!employees) {
        fprintf(stderr, "Memory allocation failed for employees\n");
        sqlite3_close(db);
        return &(api_response_t){.body="Internal Server Error", .code=500};
    }

    if (service_select_employees_paginated(db, page, page_size, employees, &count) < 0) {
        fprintf(stderr, "Failed to select employees from database\n");
        free(employees);
        sqlite3_close(db);
        return &(api_response_t){.body="Internal Server Error", .code=500};
    }

    size_t body_size = page_size * EMPLOYEE_SIZE;
    char* body = malloc(body_size);
    
    if (!body) {
        fprintf(stderr, "Memory allocation failed for response body\n");
        sqlite3_close(db);
        free(employees);
        return &(api_response_t){.body="Internal Server Error", .code=500};
    }

    snprintf(body, body_size, "[");

    for (int i = 0; i < page_size && i < count; i++) {
        char buf[EMPLOYEE_SIZE];
        snprintf(buf, sizeof(buf),
                 "{\"id\": %d, \"name\": \"%s\", \"surname\": \"%s\", \"position_id\": %d, \"role_id\": %d, \"password\": \"%s\"}%s",
                 employees[i].id, employees[i].name, employees[i].surname, employees[i].position_id, employees[i].role, employees[i].password,
                 (i < page_size - 1 && i < count - 1) ? "," : "");
        strncat(body, buf, body_size - strlen(body) - 1);
    }

    strncat(body, "]", body_size - strlen(body) - 1);
    sqlite3_close(db);

    api_response_t* response = &(api_response_t){.body = body, .code = 200};
    return response;
}

api_response_t* add_employee(void* args) {
    api_request_t* request_params = (api_request_t*)args;
    sqlite3* db = NULL;

    if (open_database(&db) < 0) {
        fprintf(stderr, "Failed to open database\n");
        return &(api_response_t){.body="Internal Server Error", .code=500};
    }

    Employee* employee = parse_employee_json(request_params->body);

    if (service_add_employee(db, employee) < 0){
        fprintf(stderr, "Failed to add employee\n");
        return &(api_response_t){.body="Internal Server Error", .code=500};
    }

    api_response_t* response = &(api_response_t){.body = "Employee created", .code = 201};
    return response;
}

api_response_t* delete_employee(void* args) {
    api_request_t* request_params = (api_request_t*)args;
    sqlite3* db = NULL;

    if (open_database(&db) < 0) {
        fprintf(stderr, "Failed to open database\n");
        return &(api_response_t){.body="Internal Server Error", .code=500};
    }

    const int id = atoi(request_params->params);

    if (service_delete_employee_by_id(db, id) < 0) {
        fprintf(stderr, "Failed to delete employee from database\n");
        sqlite3_close(db);
        return &(api_response_t){.body="Bad Request", .code=400};
    }

    sqlite3_close(db);

    api_response_t* response = &(api_response_t){.body = "Employee deleted", .code = 204};
    return response;
}

api_response_t* update_employee(void* args){
    api_request_t* request_params = (api_request_t*)args;
    sqlite3* db = NULL;

    if (open_database(&db) < 0) {
        fprintf(stderr, "Failed to open database\n");
        return &(api_response_t){.body="Internal Server Error", .code=500};
    }

    Employee* employee = parse_employee_json(request_params->body);

    if(employee->id <= 0){
        fprintf(stderr, "No id provided\n");
        return &(api_response_t){.body="Bad Request", .code=400};
    }

    if(service_update_employee(db, employee) < 0){
        fprintf(stderr, "Employee with provided ID doesn't exist\n");
        return &(api_response_t){.body="Bad Request", .code=400};
    }


    api_response_t* response = &(api_response_t){.body = "Employee updated", .code = 200};
    return response;
}

static Employee* parse_employee_json(char* json) {
    Employee* e = calloc(1, sizeof(Employee));
    char *str1, *token, *subtoken;
    char *saveptr1, *saveptr2;

    // Clearing braces
    json += 1;
    json[strlen(json) - 1] = '\0';

   for (str1 = json; ; str1 = NULL) {
        token = strtok_r(str1, ",", &saveptr1);
        if (token == NULL)
            break;

        subtoken = json_trim(strtok_r(token, ":", &saveptr2));
        char* value =  json_trim(strtok_r(NULL, ":", &saveptr2));
        
        if (strcmp("id", subtoken) == 0) e->id = atoi(value);
        else if(strcmp("name", subtoken) == 0) strcpy(e->name, value);
        else if(strcmp("surname", subtoken) == 0) strcpy(e->surname, value);
        else if(strcmp("position_id", subtoken) == 0) e->position_id = atoi(value);
        else if(strcmp("role_id", subtoken) == 0) e->role = atoi(value);
        else if(strcmp("password", subtoken) == 0) strcpy(e->password, value);
    }

    return e;
}

