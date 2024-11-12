#include "mqtt_thr.h"

void delivered(void *context, MQTTClient_deliveryToken dt) {
    (void)context;
    printf("Message with token value %d delivery confirmed\n", dt);
}

void connection_lost(void *context, char *cause) {
    mqtt_client_t *client = (mqtt_client_t *)context;
    printf("\nConnection lost: %s\n", cause);
    client->connected = 0;
}

int on_message(void *context, char *topicName, int topicLen, MQTTClient_message *message) {
    (void)topicLen;
    mqtt_client_t *client = (mqtt_client_t *)context;
    
    pthread_mutex_lock(&client->mutex);
    
    char *payload = malloc(message->payloadlen + 1);
    memcpy(payload, message->payload, message->payloadlen);
    payload[message->payloadlen] = '\0';
    
    time_t now;
    time(&now);
    char *time_str = ctime(&now);
    time_str[strlen(time_str)-1] = '\0';
    
    printf("\n[%s]\n", time_str);
    printf("Topic: %s\n", topicName);
    printf("Message: %s\n", payload);
    
    // Parse JSON and trigger memory search
    cJSON *json = cJSON_Parse(payload);
    if (json != NULL) {
        cJSON *check = cJSON_GetObjectItem(json, "check");
        if (check != NULL && cJSON_IsString(check)) {
            printf("Check command: %s\n", check->valuestring);
            
            // Send request to memory search thread
            pthread_mutex_lock(&client->mem_info->mutex);
            strncpy(client->mem_info->search_key, check->valuestring, sizeof(client->mem_info->search_key) - 1);
            client->mem_info->has_new_request = 1;
            pthread_cond_signal(&client->mem_info->cond);
            pthread_mutex_unlock(&client->mem_info->mutex);
        }
        cJSON_Delete(json);
    }
    
    free(payload);
    pthread_mutex_unlock(&client->mutex);
    
    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    
    return 1;
}

void *mqtt_client_thread(void *arg) {
    mqtt_client_t *client = (mqtt_client_t *)arg;
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    int retry_count = 0;
    const int max_retries = 5;
    
    // Set connection options
    conn_opts.keepAliveInterval = 60;
    conn_opts.cleansession = 1;
    conn_opts.connectTimeout = 5;
    
    // Set callbacks
    MQTTClient_setCallbacks(client->client, client, connection_lost, 
                           on_message, delivered);
    
    // Connect to broker with retry
    while (retry_count < max_retries) {
        printf("Attempting to connect to EMQX broker (attempt %d/%d)...\n", 
               retry_count + 1, max_retries);
        
        int rc = MQTTClient_connect(client->client, &conn_opts);
        if (rc == MQTTCLIENT_SUCCESS) {
            printf("Successfully connected to EMQX broker\n");
            client->connected = 1;
            break;
        } else {
            printf("Failed to connect, return code %d\n", rc);
            retry_count++;
            sleep(5);
        }
    }
    
    if (!client->connected) {
        printf("Failed to connect after %d attempts\n", max_retries);
        return NULL;
    }
    
    // Subscribe to base topic with wildcard
    char subscribe_topic[128];
    snprintf(subscribe_topic, sizeof(subscribe_topic), "%s#", BASE_TOPIC);
    
    int rc = MQTTClient_subscribe(client->client, subscribe_topic, QOS);
    if (rc == MQTTCLIENT_SUCCESS) {
        printf("Subscribed to topic: %s\n", subscribe_topic);
    } else {
        printf("Failed to subscribe to topic: %s (rc=%d)\n", subscribe_topic, rc);
        MQTTClient_disconnect(client->client, TIMEOUT);
        return NULL;
    }
    
    // Keep thread running and handle reconnection
    while (1) {
        if (!client->connected) {
            printf("Connection lost. Attempting to reconnect...\n");
            if (MQTTClient_connect(client->client, &conn_opts) == MQTTCLIENT_SUCCESS) {
                printf("Reconnected to EMQX broker\n");
                MQTTClient_subscribe(client->client, subscribe_topic, QOS);
                client->connected = 1;
            }
        }
        sleep(1);
    }
    
    return NULL;
}