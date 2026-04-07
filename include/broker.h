//
// Created by jtwears on 3/25/26.
//

#ifndef JRON_BROKER_H
#define JRON_BROKER_H

#include <bits/pthreadtypes.h>
#include "topic.h"
// ============================================================================
// MESSAGE BROKER - Topic orchestrator
// ============================================================================
typedef struct {
    topic_t **topics;
    int topic_count;
    int topics_capacity;

    pthread_rwlock_t registry_lock;
} message_broker_t;

// Lifecycle
int broker_init(message_broker_t *broker);
void broker_destroy(message_broker_t *broker);

// Topic CRUD
int broker_create_topic(message_broker_t *broker, const char *name,
                       size_t buffer_capacity);
int broker_delete_topic(message_broker_t *broker, const char *name);
topic_t* broker_get_topic(message_broker_t *broker, const char *name);

// Publish (writes to topic's ring buffer, returns subscribers)
int broker_publish(message_broker_t *broker, const char *topic_name,
                   const void *data, size_t len,
                   int **out_subscribers, int *out_count);

// Broadcast

// Subscribe/unsubscribe
int broker_subscribe(message_broker_t *broker, const char *topic_name, int client_fd);
int broker_unsubscribe(message_broker_t *broker, const char *topic_name, int client_fd);

// On client disconnect: remove from all topics
int broker_client_disconnected(message_broker_t *broker, int client_fd);

#endif //JRON_BROKER_H