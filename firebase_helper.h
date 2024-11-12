#ifndef FIREBASE_HELPER_H
#define FIREBASE_HELPER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <cjson/cJSON.h>
#include "db_helper.h"

// Firebase configuration
#define FIREBASE_URL "https://hardware-79c57-default-rtdb.asia-southeast1.firebasedatabase.app/memory_data.json"

typedef struct {
    char *data;
    size_t size;
} curl_response_t;

void *firebase_thread(void *arg);
int send_to_firebase(const char *command, long value, const char *timestamp);

#endif