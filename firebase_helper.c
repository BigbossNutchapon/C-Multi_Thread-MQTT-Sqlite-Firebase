#include "firebase_helper.h"

size_t curl_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    curl_response_t *resp = (curl_response_t *)userp;
    
    char *ptr = realloc(resp->data, resp->size + realsize + 1);
    if (!ptr) {
        fprintf(stderr, "Out of memory!\n");
        return 0;
    }
    
    resp->data = ptr;
    memcpy(&(resp->data[resp->size]), contents, realsize);
    resp->size += realsize;
    resp->data[resp->size] = 0;
    
    return realsize;
}

int send_to_firebase(const char *command, long value, const char *timestamp) {
    CURL *curl;
    CURLcode res;
    curl_response_t resp = {0};
    
    cJSON *root = cJSON_CreateObject();
    cJSON *data = cJSON_CreateObject();
    cJSON_AddStringToObject(data, "command", command);
    cJSON_AddNumberToObject(data, "value", value);
    if (timestamp) {
        cJSON_AddStringToObject(data, "timestamp", timestamp);
    }
    
    time_t now;
    time(&now);
    char key[32];
    snprintf(key, sizeof(key), "data_%ld", now);
    cJSON_AddItemToObject(root, key, data);
    
    char *json_str = cJSON_Print(root);
    printf("Sending to Firebase: %s\n", json_str);
    
    curl = curl_easy_init();
    if (curl) {
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        
        curl_easy_setopt(curl, CURLOPT_URL, FIREBASE_URL);
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PATCH");
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_str);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&resp);
        
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        
        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", 
                    curl_easy_strerror(res));
            return -1;
        }
        
        if (resp.data) {
            printf("Firebase Response: %s\n", resp.data);
        }
        
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    }
    
    cJSON_Delete(root);
    free(json_str);
    if (resp.data) free(resp.data);
    
    return 0;
}

void *firebase_thread(void *arg) {
    sqlite3 *db;
    sqlite3_stmt *stmt;
    int last_id = 0;
    
    const char *query = "SELECT _id, command, value, timestamp FROM mem_table WHERE _id > ? ORDER BY _id";
    
    while (1) {
        if (sqlite3_open(DB_NAME, &db) == SQLITE_OK) {
            if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) == SQLITE_OK) {
                sqlite3_bind_int(stmt, 1, last_id);
                
                while (sqlite3_step(stmt) == SQLITE_ROW) {
                    int id = sqlite3_column_int(stmt, 0);
                    const char *command = (const char *)sqlite3_column_text(stmt, 1);
                    long value = sqlite3_column_int64(stmt, 2);
                    const char *timestamp = (const char *)sqlite3_column_text(stmt, 3);
                    
                    printf("Attempting to send to Firebase - ID: %d, Command: %s, Value: %ld\n", 
                           id, command, value);
                           
                    if (send_to_firebase(command, value, timestamp) == 0) {
                        printf("Successfully sent to Firebase - Command: %s, Value: %ld\n", 
                               command, value);
                        last_id = id;
                    } else {
                        printf("Failed to send to Firebase - ID: %d\n", id);
                    }
                }
                
                sqlite3_finalize(stmt);
            }
            sqlite3_close(db);
        }
        
        sleep(5);
    }
    
    return NULL;
}