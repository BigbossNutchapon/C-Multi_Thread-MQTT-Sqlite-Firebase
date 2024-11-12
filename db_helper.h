#ifndef DB_HELPER_H
#define DB_HELPER_H

// include files
#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <string.h>

// Constants
#define DB_NAME "/home/useros/workshop/mqtt/mem.db"

// function prototypes
void dbase_init(const char *db_name);
int dbase_append(const char *db_name, const char *command, int value);
int dbase_query(const char *db_name);

#endif