#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "config/routes.h"
#include "controllers/employeeController.h"

void setup_routes(Config *cfg){
    cfg->router = init_router();

    config_router_append(cfg, "api/employee/:id", HTTP_GET, select_employee);
    config_router_append(cfg, "api/employees", HTTP_GET, select_employees);
};