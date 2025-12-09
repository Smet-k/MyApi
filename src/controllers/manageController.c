#include "controllers/manageController.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "config/db.h"
#include "services/employeeService.h"
#include "services/positionService.h"

#define PAGE_SIZE 10
#define EMPLOYEE_SIZE 512
static void json_remove_spaces(char *s);
static Employee* parse_employee_json(char* json);

api_response_t* select_employees_view(void* args){
    api_request_t* request_params = (api_request_t*)args;
    sqlite3* db = NULL;
    if (open_database(&db) < 0) {
        fprintf(stderr, "Failed to open database\n");
        return &(api_response_t){.reason="Internal Server Error", .code=500};
    }

    char search[50];
    int page = 1;
    int page_size = PAGE_SIZE;

    parse_paginated_with_search(request_params->query, &page, &page_size, search);
    paginated_request_t request = {.page=page, .page_size=page_size};

    Employee* employees = calloc(page_size, sizeof(Employee) * page_size);

    if (!employees) {
        fprintf(stderr, "Memory allocation failed for employees\n");
        sqlite3_close(db);
        return &(api_response_t){.reason="Internal Server Error", .code=500};
    }

    if (service_select_employees_paginated(db, &request, search, employees) < 0) {
        fprintf(stderr, "Failed to select employees from database\n");
        free(employees);
        sqlite3_close(db);
        return &(api_response_t){.reason="Internal Server Error", .code=500};
    }

    size_t body_size = page_size * EMPLOYEE_SIZE;
    char* body = malloc(body_size);
    
    if (!body) {
        fprintf(stderr, "Memory allocation failed for response body\n");
        sqlite3_close(db);
        free(employees);
        return &(api_response_t){.reason="Internal Server Error", .code=500};
    }

    snprintf(body, body_size, "{ \"items\": [");

    for (int i = 0; i < request.count; i++) {
        char buf[EMPLOYEE_SIZE];
        Position pos;
        service_select_position_by_id(db, employees[i].position_id, &pos);

        snprintf(buf, sizeof(buf),
                 "{\"id\": %d, \"login\": \"%s\", \"name\": \"%s\", \"surname\": \"%s\", \"employment_date\": \"%s\", \"position_id\": %d, \"role_id\": %d, \"password\": \"%s\", \"position\": \"%s\"}%s",
                 employees[i].id, employees[i].login, employees[i].name, employees[i].surname, employees[i].date, employees[i].position_id, employees[i].role, employees[i].password, pos.title,
                 (i < page_size - 1 && i < request.count - 1) ? "," : "");
        strncat(body, buf, body_size - strlen(body) - 1);
    }

    strncat(body, "], ", body_size - strlen(body) - 1);
    char count_buf[64];
    snprintf(count_buf, sizeof(count_buf), "\"total_count\": %d", request.total_count);
    strncat(body, count_buf, body_size - strlen(body) - 1);

    strncat(body, "}", body_size - strlen(body) - 1);

    sqlite3_close(db);

    api_response_t* response = &(api_response_t){.body = body, .code = 200, .reason="OK"};
    return response;
}

api_response_t* auth_employee(void* args){
    api_request_t* request_params = (api_request_t*)args;
    sqlite3* db = NULL;

    if (open_database(&db) < 0) {
        fprintf(stderr, "Failed to open database\n");
        return &(api_response_t){.reason="Internal Server Error", .code=500};
    }

    json_remove_spaces(request_params->body);
    Employee* employee = parse_employee_json(request_params->body);
    char* password = employee->password;

    if(service_select_employee_by_login(db, employee->login,employee) < 0){
        fprintf(stderr, "User with provided login doesn't exist\n");
        return &(api_response_t){.reason="Bad Request", .code=400};
    }

    if (strcmp(password, employee->password)){
        printf("User not authorized\n");
        return &(api_response_t){.reason="Unauthorized", .code=401};
    }

    Position pos;
    service_select_position_by_id(db, employee->position_id, &pos);


    char body[EMPLOYEE_SIZE + 64];
    snprintf(body, sizeof(body),
             "{\"id\": %d, \"login\": \"%s\", \"name\": \"%s\", \"surname\": \"%s\", \"employment_date\": \"%s\", \"position_id\": %d, \"role_id\": %d, \"password\": \"%s\", \"position\": \"%s\"}",
             employee->id, employee->login, employee->name, employee->surname,employee->date, employee->position_id, employee->role, employee->password, pos.title);

    api_response_t* response = &(api_response_t){.body = body, .code = 200, .reason="OK"};
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
        else if(strcmp("login", subtoken) == 0) strcpy(e->login, value);
        else if(strcmp("name", subtoken) == 0) strcpy(e->name, value);
        else if(strcmp("surname", subtoken) == 0) strcpy(e->surname, value);
        else if(strcmp("position_id", subtoken) == 0) e->position_id = atoi(value);
        else if(strcmp("role_id", subtoken) == 0) e->role = atoi(value);
        else if(strcmp("password", subtoken) == 0) strcpy(e->password, value);
        else if(strcmp("employment_date", subtoken) == 0) strcpy(e->date, value);
    }

    return e;
}

static void json_remove_spaces(char *s) {
    int in_quotes = 0, j = 0;
    for (int i = 0; s[i]; i++) {
        if (s[i] == '"') in_quotes = !in_quotes;
        if (!in_quotes && (s[i] == ' ' || s[i] == '\n' || s[i] == '\t'))
            continue;
        s[j++] = s[i];
    }
    s[j] = '\0';
}