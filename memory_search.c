#include "mqtt_thr.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "db_helper.h"

long search_meminfo(const char *search_key) {
    FILE *fp;
    char line[256];
    char key[50];
    long value;
    
    fp = fopen("/proc/meminfo", "r");
    if (fp == NULL) {
        printf("Error opening /proc/meminfo\n");
        return -1;
    }
    
    while (fgets(line, sizeof(line), fp)) {
        if (sscanf(line, "%[^:]: %ld", key, &value) == 2) {
            if (strcmp(key, search_key) == 0) {
                fclose(fp);
                return value;
            }
        }
    }
    
    fclose(fp);
    return -1;
}

void *memory_search_thread(void *arg) {
    mem_info_t *mem_info = (mem_info_t *)arg;
    
    while (1) {
        pthread_mutex_lock(&mem_info->mutex);
        while (!mem_info->has_new_request) {
            pthread_cond_wait(&mem_info->cond, &mem_info->mutex);
        }
        
        char search_key[50];
        strcpy(search_key, mem_info->search_key);
        
        mem_info->has_new_request = 0;
        pthread_mutex_unlock(&mem_info->mutex);
        
        long value = search_meminfo(search_key);
        
        pthread_mutex_lock(&mem_info->mutex);
        mem_info->value = value;
        const char db_name[] = "/home/useros/workshop/mqtt/mem.db";
        dbase_init(db_name);
        dbase_append(db_name, search_key, value);
        printf("Memory Search Result - %s: %ld kB\n", search_key, value);
        pthread_mutex_unlock(&mem_info->mutex);
    }
    
    return NULL;
}