//
// Created by jtwears on 3/26/26.
//

#include "topic_dictionary.h"
#include "broker.h"
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

int broker_init(message_broker_t *broker) {
    if (!broker) return -1;
    broker->topics = NULL;
    broker->topic_count = 0;
    broker->topics_capacity = 0;
    pthread_rwlock_init(&broker->registry_lock, NULL);
    return 0;
}

void broker_destroy(message_broker_t *broker) {
    if (!broker) return;
    pthread_rwlock_wrlock(&broker->registry_lock);
    for (int i = 0; i < broker->topic_count; i++) {
        topic_destroy(broker->topics[i]);
    }
    free(broker->topics);
    broker->topics = NULL;
    broker->topic_count = 0;
    broker->topics_capacity = 0;
    pthread_rwlock_unlock(&broker->registry_lock);
    pthread_rwlock_destroy(&broker->registry_lock);
}

int broker_create_topic(message_broker_t *broker, const char *name,
                       const size_t buffer_capacity) {
    if (!broker || !name) return -1;
    pthread_rwlock_wrlock(&broker->registry_lock);
    // Check if topic already exists
    for (int i = 0; i < broker->topic_count; i++) {
        if (strcmp(broker->topics[i]->name, name) == 0) {
            pthread_rwlock_unlock(&broker->registry_lock);
            return -1; // Topic already exists
        }
    }
    // Create new topic
    topic_t *new_topic = topic_create(name, buffer_capacity, sizeof(char*)); // Assuming string messages
    if (!new_topic) {
        pthread_rwlock_unlock(&broker->registry_lock);
        return -1; // Failed to create topic
    }
    // Add to broker's topic list
    if (broker->topic_count == broker->topics_capacity) {
        const int new_capacity = broker->topics_capacity == 0 ? 4 : broker->topics_capacity * 2;
        topic_t **new_topics = realloc(broker->topics, new_capacity * sizeof(topic_t*));
        if (!new_topics) {
            topic_destroy(new_topic);
            pthread_rwlock_unlock(&broker->registry_lock);
            return -1; // Failed to resize topic list
        }
        broker->topics = new_topics;
        broker->topics_capacity = new_capacity;
    }
    broker->topics[broker->topic_count++] = new_topic;
    pthread_rwlock_unlock(&broker->registry_lock);
    return 0;
}


int broker_delete_topic(message_broker_t *broker, const char *name) {
    if (!broker || !name) return -1;
    pthread_rwlock_wrlock(&broker->registry_lock);
    for (int i = 0; i < broker->topic_count; i++) {
        if (strcmp(broker->topics[i]->name, name) == 0) {
            topic_destroy(broker->topics[i]);
            // Shift remaining topics down
            memmove(&broker->topics[i], &broker->topics[i + 1], (broker->topic_count - i - 1) * sizeof(topic_t*));
            broker->topic_count--;
            pthread_rwlock_unlock(&broker->registry_lock);
            return 0; // Topic deleted
        }
    }
    pthread_rwlock_unlock(&broker->registry_lock);
    return -1; // Topic not found
}

topic_t* broker_get_topic(message_broker_t *broker, const char *name) {
    if (!broker || !name) return NULL;
    pthread_rwlock_rdlock(&broker->registry_lock);
    for (int i = 0; i < broker->topic_count; i++) {
        if (strcmp(broker->topics[i]->name, name) == 0) {
            pthread_rwlock_unlock(&broker->registry_lock);
            return broker->topics[i]; // Topic found
        }
    }
    pthread_rwlock_unlock(&broker->registry_lock);
    return NULL; // Topic not found
}

int broker_publish(message_broker_t *broker, const char *topic_name,
                   const void *data, size_t len,
                   int **out_subscribers, int *out_count) {
    if (!broker || !topic_name || !data || !out_subscribers || !out_count) return -1;
    topic_t *topic = broker_get_topic(broker, topic_name);
    if (!topic) return -1; // Topic not found
    const int res = topic_publish(topic, data, len);
    if (res < 0) return res; // Failed to publish
    // Return subscribers
    pthread_mutex_lock(&topic->sub_lock);
    *out_count = topic->subscriber_count;
    *out_subscribers = malloc(*out_count * sizeof(int));
    if (*out_subscribers) {
        memcpy(*out_subscribers, topic->subscribers_fds, *out_count * sizeof(int));
    }
    pthread_mutex_unlock(&topic->sub_lock);
    return 0;
}

int broker_subscribe(message_broker_t *broker, const char *topic_name, const int client_fd) {
    if (!broker || !topic_name) return -1;
    topic_t *topic = broker_get_topic(broker, topic_name);
    if (!topic) return -1; // Topic not found
    return topic_subscribe(topic, client_fd);
}

int broker_unsubscribe(message_broker_t *broker, const char *topic_name, const int client_fd) {
    if (!broker || !topic_name) return -1;
    topic_t *topic = broker_get_topic(broker, topic_name);
    if (!topic) return -1; // Topic not found
    return topic_unsubscribe(topic, client_fd);
}

int broker_client_disconnected(message_broker_t *broker, const int client_fd) {
    if (!broker) return -1;
    pthread_rwlock_rdlock(&broker->registry_lock);
    for (int i = 0; i < broker->topic_count; i++) {
        topic_unsubscribe(broker->topics[i], client_fd);
    }
    pthread_rwlock_unlock(&broker->registry_lock);
    return 0;
}
