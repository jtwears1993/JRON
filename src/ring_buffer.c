#include "ring_buffer.h"
#include <stdlib.h>
#include <string.h>

jron_ring_buffer_t* jron_ring_buffer_create(const size_t capacity, const size_t element_size) {
    if (capacity == 0 || element_size == 0) return NULL;
    jron_ring_buffer_t* rb = malloc(sizeof(jron_ring_buffer_t));
    if (!rb) return NULL;
    rb->capacity = capacity;
    rb->element_size = element_size;
    rb->head = 0;
    rb->tail = 0;
    rb->count = 0;
    rb->buffer = malloc(capacity * element_size);
    if (!rb->buffer) {
        free(rb);
        return NULL;
    }
    return rb;
}

void jron_ring_buffer_destroy(jron_ring_buffer_t* rb) {
    if (!rb) return;
    free(rb->buffer);
    free(rb);
}

int jron_ring_buffer_push(jron_ring_buffer_t* rb, const void* element) {
    if (!rb || !element) return -1;
    if (rb->count == rb->capacity) {
        // overwrite oldest (circular behavior) - for a bounded log you might choose to fail
        // here we advance tail to drop oldest
        rb->tail = (rb->tail + 1) % rb->capacity;
        rb->count = rb->capacity - 1; // will be incremented below
    }
    void* dest = (char*)rb->buffer + (rb->head * rb->element_size);
    memcpy(dest, element, rb->element_size);
    rb->head = (rb->head + 1) % rb->capacity;
    if (rb->count < rb->capacity) rb->count++;
    return 0;
}

int jron_ring_buffer_pop(jron_ring_buffer_t* rb, void* out_element) {
    if (!rb || !out_element) return -1;
    if (rb->count == 0) return -1; // empty
    const void* src = (char*)rb->buffer + (rb->tail * rb->element_size);
    memcpy(out_element, src, rb->element_size);
    rb->tail = (rb->tail + 1) % rb->capacity;
    rb->count--;
    return 0;
}

size_t jron_ring_buffer_size(const jron_ring_buffer_t* rb) {
    if (!rb) return 0;
    return rb->count;
}

int jron_ring_buffer_is_empty(const jron_ring_buffer_t* rb) {
    if (!rb) return 1;
    return rb->count == 0;
}
