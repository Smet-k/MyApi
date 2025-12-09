#ifndef EMPLOYEE_CONTROLLER_H
#define EMPLOYEE_CONTROLLER_H
#include "api_response.h"

api_response_t* select_employee(void* args);
api_response_t* select_employees(void* args);
api_response_t* add_employee(void* args);
api_response_t* delete_employee(void* args);
api_response_t* update_employee(void* args);

#endif
