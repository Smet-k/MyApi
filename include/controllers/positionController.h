#ifndef POSITION_CONTROLLER_H
#define POSITION_CONTROLLER_H
#include "api_response.h"

api_response_t* select_position(void* args);
api_response_t* select_positions(void* args);
api_response_t* add_position(void* args);
api_response_t* delete_position(void* args);
api_response_t* update_position(void* args);

#endif