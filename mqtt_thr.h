#ifndef MQTT_THR_H
#define MQTT_THR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <MQTTClient.h>
#include <pthread.h>
#include <cjson/cJSON.h>
#include <time.h>

// MQTT Configuration
#define MQTT_BROKER     "tcp://broker.emqx.io:1883"
#define MQTT_CLIENTID   "TGR2024_Nutch_123456"
#define BASE_TOPIC      "tgr2024/ece-flyhigh/check-info/"
#define QOS            1
#define TIMEOUT        10000L
#define MAX_BUFFER_SIZE 1024

// Structure for sharing data between threads
typedef struct {
    char search_key[50];
    long value;
    int has_new_request;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} mem_info_t;

// MQTT client structure
typedef struct {
    MQTTClient client;
    pthread_mutex_t mutex;
    volatile int connected;
    mem_info_t *mem_info;
} mqtt_client_t;

// Function declarations for mqtt thread
void *mqtt_client_thread(void *arg);
void delivered(void *context, MQTTClient_deliveryToken dt);
void connection_lost(void *context, char *cause);
int on_message(void *context, char *topicName, int topicLen, MQTTClient_message *message);

// Function declarations for memory thread
void *memory_search_thread(void *arg);
long search_meminfo(const char *search_key);

#endif // MQTT_THR_H