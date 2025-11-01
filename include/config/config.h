#ifndef CONFIG_H
#define CONFIG_H
#include "router.h"

typedef struct {
    int port;
    int threads;
    router_t* router;
} Config;

int load_config(const char* filename, Config* config);
void config_router_append(Config* cfg, char* path, http_method_t method, void* function);

#endif