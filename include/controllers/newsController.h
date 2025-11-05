#ifndef NEWS_CONTROLLER_H
#define NEWS_CONTROLLER_H
#include "api_response.h"

api_response_t* select_newsletter(void* args);
api_response_t* select_news(void* args);
api_response_t* add_news(void* args);
api_response_t* delete_news(void* args);
api_response_t* update_news(void* args);

#endif