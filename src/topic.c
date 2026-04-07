//
// Created by jtwears on 3/26/26.
//

#include "topic.h"
#include "ring_buffer.h"
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <asm-generic/errno-base.h>

topic_t *topic_create(const char *name, const size_t size, const size_t element_size) {
    topic_t *t = calloc(1, sizeof(*t));
    if (!t) return NULL;

    if (name) {
        t->name = strdup(name);
        if (!t->name) {
            free(t);
            return NULL;
        }
    } else {
        t->name = NULL;
    }

    /* initialize ring buffer as appropriate for your API.
       If your ring buffer is value-typed, initialize here.
       Replace the following with your actual init call if needed. */
    if (jron_ring_buffer_create(size, element_size) == NULL) {
        free(t->name);
        free(t);
        return NULL;
    }

    pthread_mutex_init(&t->sub_lock, NULL);
    t->subscriber_count = 0;
    return t;
}

void topic_destroy(topic_t *t) {
    if (t) {
        jron_ring_buffer_destroy(&t->buffer);
        pthread_mutex_lock(&t->sub_lock);
        t->subscriber_count = 0;
        pthread_mutex_unlock(&t->sub_lock);
        pthread_mutex_destroy(&t->sub_lock);
        free(t->name);
        free(t);
    }
}

int topic_publish(topic_t *t, const void *data, size_t len) {
    if (!t) return EINVAL;

    if (!data) return EINVAL;

    const int res = jron_ring_buffer_push(&t->buffer, data);
    if (res < 0) return res;
    return 0;
}

int topic_has_messages(topic_t *t) {
    if (!t) return 0;
    return !jron_ring_buffer_is_empty(&t->buffer);
}

int topic_read_next(topic_t *t, void *out_buffer, const size_t buffer_size) {
    if (!t || !out_buffer || buffer_size == 0) return EINVAL;

    if (jron_ring_buffer_is_empty(&t->buffer)) return 0; // no messages

    const int res = jron_ring_buffer_pop(&t->buffer, out_buffer);
    if (res < 0) return res;
    return 1; // success
}

int topic_subscribe(topic_t *t, const int client_fd) {

    if (!t) return EINVAL;

    // lock as we need to read
    pthread_mutex_lock(&t->sub_lock);
    if (t->subscriber_count >= MAX_SUBSCRIBERS) {
        pthread_mutex_unlock(&t->sub_lock);
        return -1; // max subscribers reached
    }

    t->subscribers_fds[t->subscriber_count++] = client_fd;
    pthread_mutex_unlock(&t->sub_lock);
    return 0;
}

int topic_unsubscribe(topic_t *t, const int client_fd) {
    if (!t) return EINVAL;

    // lock as we need to read
    pthread_mutex_lock(&t->sub_lock);
    // Find and remove the client_fd and
    for (int i = 0; i < t->subscriber_count; ++i) {
        if (t->subscribers_fds[i] == client_fd) {
            // Shift remaining fds down
            for (int j = i; j < t->subscriber_count - 1; ++j) {
                t->subscribers_fds[j] = t->subscribers_fds[j + 1];
            }
            t->subscriber_count--;
            pthread_mutex_lock(&t->sub_lock);
            return 0; // success
        }
    }
    pthread_mutex_unlock(&t->sub_lock);
    return -ENOENT;
}

const int* topic_get_subscribers(topic_t *t, int *out_count) {
    if (!t || !out_count) return NULL;
    pthread_mutex_lock(&t->sub_lock);
    *out_count = t->subscriber_count;
    pthread_mutex_unlock(&t->sub_lock);
    return t->subscribers_fds;
}
