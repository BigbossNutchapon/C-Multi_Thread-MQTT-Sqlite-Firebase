#include "mqtt_thr.h"
#include "firebase_helper.h"

int main(void) {
    mqtt_client_t client = {0};
    mem_info_t mem_info = {0};
    int rc;
    
    // Initialize mutexes and condition variables
    pthread_mutex_init(&client.mutex, NULL);
    pthread_mutex_init(&mem_info.mutex, NULL);
    pthread_cond_init(&mem_info.cond, NULL);
    
    // Connect mem_info to client
    client.mem_info = &mem_info;
    
    // Initialize curl library
    curl_global_init(CURL_GLOBAL_ALL);
    
    // Create MQTT client
    if ((rc = MQTTClient_create(&client.client, MQTT_BROKER, MQTT_CLIENTID,
        MQTTCLIENT_PERSISTENCE_NONE, NULL)) != MQTTCLIENT_SUCCESS) {
        printf("Failed to create client, return code %d\n", rc);
        return EXIT_FAILURE;
    }
    
    // Create threads
    pthread_t memory_thread_id, mqtt_thread_id, firebase_thread_id;
    
    // Create memory search thread
    rc = pthread_create(&memory_thread_id, NULL, memory_search_thread, &mem_info);
    if (rc != 0) {
        printf("Failed to create memory search thread\n");
        return EXIT_FAILURE;
    }
    
    // Create MQTT thread
    rc = pthread_create(&mqtt_thread_id, NULL, mqtt_client_thread, &client);
    if (rc != 0) {
        printf("Failed to create MQTT thread\n");
        MQTTClient_destroy(&client.client);
        return EXIT_FAILURE;
    }
    
    // Create Firebase thread
    rc = pthread_create(&firebase_thread_id, NULL, firebase_thread, NULL);
    if (rc != 0) {
        printf("Failed to create Firebase thread\n");
        return EXIT_FAILURE;
    }
    
    pthread_join(mqtt_thread_id, NULL);
    pthread_join(memory_thread_id, NULL);
    pthread_join(firebase_thread_id, NULL);
    
    pthread_mutex_destroy(&client.mutex);
    pthread_mutex_destroy(&mem_info.mutex);
    pthread_cond_destroy(&mem_info.cond);
    MQTTClient_disconnect(client.client, TIMEOUT);
    MQTTClient_destroy(&client.client);
    curl_global_cleanup();
    
    return EXIT_SUCCESS;
}