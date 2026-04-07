//
// Created by jtwears on 3/25/26.
//

#ifndef JRON_TOPIC_H
#define JRON_TOPIC_H
#include <stddef.h>
#include <bits/pthreadtypes.h>

#include "ring_buffer.h"

// ============================================================================
// TOPIC - Pub/sub semantics, but NO per-client reading
// ============================================================================

#define MAX_SUBSCRIBERS 20

typedef struct {
    char *name;
    jron_ring_buffer_t buffer;

    int subscriber_count;
    // track subscribers by file descriptor for quick removal on disconnect
    int subscribers_fds[MAX_SUBSCRIBERS];

    pthread_mutex_t sub_lock;
} topic_t;

// Lifecycle
topic_t *topic_create(const char *name, size_t size, size_t element_size);
void topic_destroy(topic_t *t);

// Publishing (write to ring buffer)
int topic_publish(topic_t *t, const void *data, size_t len);

// Get next message for a subscriber (returns 0 if no messages)
int topic_read_next(topic_t *t, void *out_buffer, size_t buffer_size);

int topic_has_messages(topic_t *t);

// Subscription management
int topic_subscribe(topic_t *t, int client_fd);
int topic_unsubscribe(topic_t *t, int client_fd);

// Get current subscribers
const int* topic_get_subscribers(topic_t *t, int *out_count);

#endif //JRON_TOPIC_H