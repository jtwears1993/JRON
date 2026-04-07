#ifndef JRON_MESSAGE_H
#define JRON_MESSAGE_H

#include <stdint.h>
#include <stddef.h>
typedef struct __attribute__((aligned(8))) {
    uint64_t offset;
    uint64_t timestamp;
    size_t   length;
    void*    payload; /* payload follows or points to external buffer */
} jron_message_t;

#endif // JRON_MESSAGE_H#endif // JRON_MESSAGE_H

