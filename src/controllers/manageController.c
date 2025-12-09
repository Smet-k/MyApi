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
