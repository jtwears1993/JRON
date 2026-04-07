#ifndef JRON_RING_BUFFER_H
#define JRON_RING_BUFFER_H

#include <stddef.h>

typedef struct jron_ring_buffer {
    size_t capacity;
    size_t element_size;
    size_t head; // next write index
    size_t tail; // next read index
    size_t count;
    void* buffer; // raw storage
} jron_ring_buffer_t;

jron_ring_buffer_t* jron_ring_buffer_create(size_t capacity, size_t element_size);
void jron_ring_buffer_destroy(jron_ring_buffer_t* rb);
int jron_ring_buffer_push(jron_ring_buffer_t* rb, const void* element);
int jron_ring_buffer_pop(jron_ring_buffer_t* rb, void* out_element);
size_t jron_ring_buffer_size(const jron_ring_buffer_t* rb);
int jron_ring_buffer_is_empty(const jron_ring_buffer_t* rb);

#endif // JRON_RING_BUFFER_H
