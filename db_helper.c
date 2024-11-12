#include "db_helper.h"

// private constants
const char INIT_SQL_CMD[] = "CREATE TABLE IF NOT EXISTS mem_table( \
                                    _id INTEGER PRIMARY KEY AUTOINCREMENT, \
                                    command VARCHAR(255), \
                                    value INTEGER, \
                                    timestamp DATETIME DEFAULT CURRENT_TIMESTAMP \
                                );";

const char APPEND_SQL_CMD[] = "INSERT INTO mem_table (command, value) VALUES (?, ?)";

const char QUERY_SQL_CMD[] = "SELECT * FROM mem_table ORDER BY timestamp DESC LIMIT 1";

// private functions

// initialize database
void dbase_init(const char *db_name) {
    sqlite3 *db;

    if (sqlite3_open(db_name, &db) != SQLITE_OK) {
        fprintf(stderr, "Error opening %s database: %s\n", db_name, sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    if (sqlite3_exec(db, INIT_SQL_CMD, NULL, NULL, NULL) != SQLITE_OK) {
        fprintf(stderr, "Error executing SQL: %s\n", sqlite3_errmsg(db)); 
        sqlite3_close(db);
        return;
    }

    sqlite3_close(db);
}

// append data to the table
int dbase_append(const char *db_name, const char *command, int value) {
    sqlite3 *db;
    sqlite3_stmt *stmt;

    if (sqlite3_open(db_name, &db) != SQLITE_OK) {
        fprintf(stderr, "Error opening %s database: %s\n", db_name, sqlite3_errmsg(db));
        sqlite3_close(db);
        return -1;
    }

    if (sqlite3_prepare_v2(db, APPEND_SQL_CMD, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Error preparing SQL: %s\n", sqlite3_errmsg(db)); 
        sqlite3_close(db);
        return -1;
    }

    sqlite3_bind_text(stmt, 1, command, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, value);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        fprintf(stderr, "Error executing SQL statement: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return -1;
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return 0;
}

// query last value from the table
int dbase_query(const char *db_name) {
    sqlite3 *db;
    sqlite3_stmt *stmt;
    int last_value = -1;

    if (sqlite3_open(db_name, &db) != SQLITE_OK) {
        fprintf(stderr, "Error opening %s database: %s\n", db_name, sqlite3_errmsg(db));
        sqlite3_close(db);
        return -1;
    }

    if (sqlite3_prepare_v2(db, QUERY_SQL_CMD, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Error preparing SQL: %s\n", sqlite3_errmsg(db)); 
        sqlite3_close(db);
        return -1;
    }

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const char *timestamp = (const char *)sqlite3_column_text(stmt, 3);
        printf("Data timestamp: %s\n", timestamp);
        last_value = sqlite3_column_int(stmt, 2);
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return last_value;
}
