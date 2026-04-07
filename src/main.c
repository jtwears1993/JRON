#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "message.h"
#include "ring_buffer.h"

int main(void) {
    printf("JRON minimal app starting...\n");

    // Create a ring buffer able to hold 4 messages
    jron_ring_buffer_t* rb = jron_ring_buffer_create(4, sizeof(jron_message_t));
    if (!rb) {
        fprintf(stderr, "Failed to create ring buffer\n");
        return 1;
    }

    // Push a sample message
    jron_message_t msg;
    msg.offset = 1;
    msg.timestamp = 0;
    msg.length = 5;
    msg.payload = strdup("hello");

    if (jron_ring_buffer_push(rb, &msg) != 0) {
        fprintf(stderr, "push failed\n");
        jron_ring_buffer_destroy(rb);
        free(msg.payload);
        return 1;
    }

    printf("Pushed message offset=%lu\n", (unsigned long)msg.offset);

    // Pop it back
    jron_message_t out;
    if (jron_ring_buffer_pop(rb, &out) == 0) {
        printf("Popped message offset=%lu length=%zu payload=%s\n",
               (unsigned long)out.offset, out.length, (char*)out.payload);
        // Note: payload pointer was copied; avoid double free here for demo
        free(out.payload);
    } else {
        printf("No message to pop\n");
    }

    jron_ring_buffer_destroy(rb);

    printf("JRON minimal app exiting\n");
    return 0;
}
