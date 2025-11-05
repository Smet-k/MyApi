#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "config/routes.h"
#include "controllers/employeeController.h"
#include "controllers/positionController.h"
#include "controllers/newsController.h"
void setup_routes(Config *cfg){
    cfg->router = init_router();

    config_router_append(cfg, "api/employees", HTTP_GET, select_employees);
    config_router_append(cfg, "api/employee/:id", HTTP_GET, select_employee);
    config_router_append(cfg, "api/employee/:id", HTTP_DELETE, delete_employee);
    config_router_append(cfg, "api/employee", HTTP_POST, add_employee);
    config_router_append(cfg, "api/employee", HTTP_PUT, update_employee);

    config_router_append(cfg, "api/positions", HTTP_GET, select_positions);
    config_router_append(cfg, "api/position/:id", HTTP_GET, select_position);
    config_router_append(cfg, "api/position/:id", HTTP_DELETE, delete_position);
    config_router_append(cfg, "api/position", HTTP_POST, add_position);
    config_router_append(cfg, "api/position", HTTP_PUT, update_position);

    config_router_append(cfg, "api/news", HTTP_GET, select_news);
    config_router_append(cfg, "api/news/:id", HTTP_GET, select_newsletter);
    config_router_append(cfg, "api/news/:id", HTTP_DELETE, delete_news);
    config_router_append(cfg, "api/news", HTTP_POST, add_news);
    config_router_append(cfg, "api/news", HTTP_PUT, update_news);
};