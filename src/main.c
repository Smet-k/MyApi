#include <stdio.h>
#include <unistd.h>

#include "server.h"
#include "config/routes.h"
#include "config/db.h"
int main() {
    Config cfg;

    if(load_config("config.cfg", &cfg) < 0){
        perror("Couldn't parse the config");
    }

    setup_routes(&cfg);

    if(load_database() < 0){
        perror("Couldn't open the database");
    }

    run_server(cfg);

    return 0;
}

