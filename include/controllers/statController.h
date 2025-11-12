#ifndef STAT_CONTROLLER_H
#define STAT_CONTROLLER_H
#include "api_response.h"

api_response_t* select_stat(void* args);
api_response_t* select_stats(void* args);
api_response_t* add_stat(void* args);
api_response_t* delete_stat(void* args);
api_response_t* update_stat(void* args);

#endif