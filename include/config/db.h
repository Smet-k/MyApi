#ifndef DB_H
#define DB_H
#include <sqlite3.h>

#define DATABASE_NAME "office.db"

int open_database(sqlite3** db); 
int load_database();

#endif