//
// Created by jtwears on 3/26/26.
//

#ifndef JRON_JRON_PROTOCOL_H
#define JRON_JRON_PROTOCOL_H

#include <stdint.h>
#include <stddef.h>
#include "message.h"

// ============================================================================
// COMMAND TYPES
// ============================================================================
typedef enum {
    CMD_PUBLISH = 1,
    CMD_SUBSCRIBE = 2,
    CMD_UNSUBSCRIBE = 3,
    CMD_CREATE_TOPIC = 4,
    CMD_DELETE_TOPIC = 5,
} command_type_t;

// ============================================================================
// RESPONSE TYPES
// ============================================================================
typedef enum {
    RESP_OK = 0,
    RESP_ERROR = 1,
    RESP_MESSAGE = 2,
} response_type_t;

// ============================================================================
// COMMAND STRUCTURES
// ============================================================================

typedef struct {
    char topic_name[64];
    jron_message_t msg;
} cmd_publish_t;

typedef struct {
    char topic_name[64];
} cmd_subscribe_t;

typedef struct {
    char topic_name[64];
} cmd_unsubscribe_t;

typedef struct {
    char topic_name[64];
    size_t buffer_capacity;
} cmd_create_topic_t;

typedef struct {
    char topic_name[64];
} cmd_delete_topic_t;

typedef union {
    cmd_publish_t publish;
    cmd_subscribe_t subscribe;
    cmd_unsubscribe_t unsubscribe;
    cmd_create_topic_t create_topic;
    cmd_delete_topic_t delete_topic;
} command_payload_t;

typedef struct {
    command_type_t type;
    command_payload_t payload;
} command_t;

// ============================================================================
// RESPONSE STRUCTURES
// ============================================================================

typedef struct {
    char message[256];
} resp_error_t;

typedef struct {
    char topic_name[64];
    jron_message_t msg;
} resp_message_t;

typedef union {
    resp_error_t error;
    resp_message_t message;
} response_payload_t;

typedef struct {
    response_type_t type;
    response_payload_t payload;
} response_t;

// ============================================================================
// WIRE FORMAT
// ============================================================================
/*
 * All multi-byte integers are in network byte order (big-endian)
 *
 * COMMAND FRAME:
 * [1 byte: command_type][variable: payload]
 *
 * PUBLISH command:
 * [1: CMD_PUBLISH]
 * [2: topic_name_len]
 * [N: topic_name]
 * [8: timestamp]
 * [8: length (payload size)]
 * [M: payload]
 *
 * SUBSCRIBE command:
 * [1: CMD_SUBSCRIBE]
 * [2: topic_name_len]
 * [N: topic_name]
 *
 * UNSUBSCRIBE command:
 * [1: CMD_UNSUBSCRIBE]
 * [2: topic_name_len]
 * [N: topic_name]
 *
 * CREATE_TOPIC command:
 * [1: CMD_CREATE_TOPIC]
 * [2: topic_name_len]
 * [N: topic_name]
 * [8: buffer_capacity]
 *
 * DELETE_TOPIC command:
 * [1: CMD_DELETE_TOPIC]
 * [2: topic_name_len]
 * [N: topic_name]
 *
 * RESPONSE FRAME:
 * [1 byte: response_type][variable: payload]
 *
 * OK response:
 * [1: RESP_OK]
 *
 * ERROR response:
 * [1: RESP_ERROR]
 * [2: error_msg_len]
 * [N: error_msg]
 *
 * MESSAGE response:
 * [1: RESP_MESSAGE]
 * [2: topic_name_len]
 * [N: topic_name]
 * [8: offset]
 * [8: timestamp]
 * [8: length (payload size)]
 * [M: payload]
 */

// ============================================================================
// PROTOCOL PARSER/SERIALIZER (Header-only, static inline)
// ============================================================================

#include <string.h>
#include <arpa/inet.h>

// Helper: read 2-byte big-endian integer
static inline uint16_t read_u16(const char *buf) {
    return ntohs(*(uint16_t *)buf);
}

// Helper: write 2-byte big-endian integer
static inline void write_u16(char *buf, const uint16_t val) {
    *(uint16_t *)buf = htons(val);
}

// Helper: read 8-byte big-endian integer
static inline uint64_t read_u64(const char *buf) {
    return be64toh(*(uint64_t *)buf);
}

// Helper: write 8-byte big-endian integer
static inline void write_u64(char *buf, const uint64_t val) {
    *(uint64_t *)buf = htobe64(val);
}

// Parse incoming bytes from client into a command
// Returns: number of bytes consumed, or -1 on parse error
// out_cmd is filled if a complete command was parsed
static inline int protocol_parse_command(const char *buffer, size_t buffer_len, command_t *out_cmd) {
    if (buffer_len < 1) return -1;

    size_t pos = 0;
    out_cmd->type = buffer[pos];
    pos++;

    switch (out_cmd->type) {
        case CMD_PUBLISH: {
            if (pos + 2 > buffer_len) return -1;
            const uint16_t topic_len = read_u16(&buffer[pos]);
            pos += 2;

            if (pos + topic_len > buffer_len) return -1;
            memcpy(out_cmd->payload.publish.topic_name, &buffer[pos], topic_len);
            out_cmd->payload.publish.topic_name[topic_len] = '\0';
            pos += topic_len;

            if (pos + 8 > buffer_len) return -1;
            out_cmd->payload.publish.msg.timestamp = read_u64(&buffer[pos]);
            pos += 8;

            if (pos + 8 > buffer_len) return -1;
            const size_t payload_len = read_u64(&buffer[pos]);
            pos += 8;

            if (pos + payload_len > buffer_len) return -1;
            out_cmd->payload.publish.msg.payload = (void *)&buffer[pos];
            out_cmd->payload.publish.msg.length = payload_len;
            pos += payload_len;

            return pos;
        }

        case CMD_SUBSCRIBE: {
            if (pos + 2 > buffer_len) return -1;
            const uint16_t topic_len = read_u16(&buffer[pos]);
            pos += 2;

            if (pos + topic_len > buffer_len) return -1;
            memcpy(out_cmd->payload.subscribe.topic_name, &buffer[pos], topic_len);
            out_cmd->payload.subscribe.topic_name[topic_len] = '\0';
            pos += topic_len;

            return pos;
        }

        case CMD_UNSUBSCRIBE: {
            if (pos + 2 > buffer_len) return -1;
            const uint16_t topic_len = read_u16(&buffer[pos]);
            pos += 2;

            if (pos + topic_len > buffer_len) return -1;
            memcpy(out_cmd->payload.unsubscribe.topic_name, &buffer[pos], topic_len);
            out_cmd->payload.unsubscribe.topic_name[topic_len] = '\0';
            pos += topic_len;

            return pos;
        }

        case CMD_CREATE_TOPIC: {
            if (pos + 2 > buffer_len) return -1;
            const uint16_t topic_len = read_u16(&buffer[pos]);
            pos += 2;

            if (pos + topic_len > buffer_len) return -1;
            memcpy(out_cmd->payload.create_topic.topic_name, &buffer[pos], topic_len);
            out_cmd->payload.create_topic.topic_name[topic_len] = '\0';
            pos += topic_len;

            if (pos + 8 > buffer_len) return -1;
            out_cmd->payload.create_topic.buffer_capacity = read_u64(&buffer[pos]);
            pos += 8;

            return pos;
        }

        case CMD_DELETE_TOPIC: {
            if (pos + 2 > buffer_len) return -1;
            const uint16_t topic_len = read_u16(&buffer[pos]);
            pos += 2;

            if (pos + topic_len > buffer_len) return -1;
            memcpy(out_cmd->payload.delete_topic.topic_name, &buffer[pos], topic_len);
            out_cmd->payload.delete_topic.topic_name[topic_len] = '\0';
            pos += topic_len;

            return pos;
        }

        default:
            return -1;
    }
}

// Serialize a response into bytes for transmission
// Returns: number of bytes written to out_buffer, or -1 on error
static inline int protocol_serialize_response(const response_t *resp, char *out_buffer, size_t out_buffer_len) {
    size_t pos = 0;

    if (pos + 1 > out_buffer_len) return -1;
    out_buffer[pos] = resp->type;
    pos++;

    switch (resp->type) {
        case RESP_OK:
            return pos;

        case RESP_ERROR: {
            const size_t msg_len = strlen(resp->payload.error.message);
            if (pos + 2 + msg_len > out_buffer_len) return -1;

            write_u16(&out_buffer[pos], msg_len);
            pos += 2;

            memcpy(&out_buffer[pos], resp->payload.error.message, msg_len);
            pos += msg_len;

            return pos;
        }

        case RESP_MESSAGE: {
            const size_t topic_len = strlen(resp->payload.message.topic_name);
            if (pos + 2 + topic_len + 8 + 8 + 8 + resp->payload.message.msg.length > out_buffer_len)
                return -1;

            write_u16(&out_buffer[pos], topic_len);
            pos += 2;

            memcpy(&out_buffer[pos], resp->payload.message.topic_name, topic_len);
            pos += topic_len;

            write_u64(&out_buffer[pos], resp->payload.message.msg.offset);
            pos += 8;

            write_u64(&out_buffer[pos], resp->payload.message.msg.timestamp);
            pos += 8;

            write_u64(&out_buffer[pos], resp->payload.message.msg.length);
            pos += 8;

            memcpy(&out_buffer[pos], resp->payload.message.msg.payload, resp->payload.message.msg.length);
            pos += resp->payload.message.msg.length;

            return pos;
        }

        default:
            return -1;
    }
}

// Serialize a command (for testing/client libraries)
static inline int protocol_serialize_command(const command_t *cmd, char *out_buffer, size_t out_buffer_len) {
    size_t pos = 0;

    if (pos + 1 > out_buffer_len) return -1;
    out_buffer[pos] = cmd->type;
    pos++;

    switch (cmd->type) {
        case CMD_PUBLISH: {
            const size_t topic_len = strlen(cmd->payload.publish.topic_name);
            if (pos + 2 + topic_len + 8 + 8 + cmd->payload.publish.msg.length > out_buffer_len)
                return -1;

            write_u16(&out_buffer[pos], topic_len);
            pos += 2;

            memcpy(&out_buffer[pos], cmd->payload.publish.topic_name, topic_len);
            pos += topic_len;

            write_u64(&out_buffer[pos], cmd->payload.publish.msg.timestamp);
            pos += 8;

            write_u64(&out_buffer[pos], cmd->payload.publish.msg.length);
            pos += 8;

            memcpy(&out_buffer[pos], cmd->payload.publish.msg.payload, cmd->payload.publish.msg.length);
            pos += cmd->payload.publish.msg.length;

            return pos;
        }

        case CMD_SUBSCRIBE:
        case CMD_UNSUBSCRIBE: {
            const size_t topic_len = strlen(cmd->payload.subscribe.topic_name);
            if (pos + 2 + topic_len > out_buffer_len) return -1;

            write_u16(&out_buffer[pos], topic_len);
            pos += 2;

            memcpy(&out_buffer[pos], cmd->payload.subscribe.topic_name, topic_len);
            pos += topic_len;

            return pos;
        }

        case CMD_CREATE_TOPIC: {
            const size_t topic_len = strlen(cmd->payload.create_topic.topic_name);
            if (pos + 2 + topic_len + 8 > out_buffer_len) return -1;

            write_u16(&out_buffer[pos], topic_len);
            pos += 2;

            memcpy(&out_buffer[pos], cmd->payload.create_topic.topic_name, topic_len);
            pos += topic_len;

            write_u64(&out_buffer[pos], cmd->payload.create_topic.buffer_capacity);
            pos += 8;

            return pos;
        }

        case CMD_DELETE_TOPIC: {
            const size_t topic_len = strlen(cmd->payload.delete_topic.topic_name);
            if (pos + 2 + topic_len > out_buffer_len) return -1;

            write_u16(&out_buffer[pos], topic_len);
            pos += 2;

            memcpy(&out_buffer[pos], cmd->payload.delete_topic.topic_name, topic_len);
            pos += topic_len;

            return pos;
        }

        default:
            return -1;
    }
}

// Helper: create an OK response
static inline response_t protocol_response_ok(void) {
    const response_t resp = {.type = RESP_OK};
    return resp;
}

// Helper: create an ERROR response
static inline response_t protocol_response_error(const char *error_msg) {
    response_t resp = {.type = RESP_ERROR};
    strncpy(resp.payload.error.message, error_msg, sizeof(resp.payload.error.message) - 1);
    resp.payload.error.message[sizeof(resp.payload.error.message) - 1] = '\0';
    return resp;
}

// Helper: create a MESSAGE response
static inline response_t protocol_response_message(const char *topic_name, const jron_message_t *msg) {
    response_t resp = {.type = RESP_MESSAGE};
    strncpy(resp.payload.message.topic_name, topic_name, sizeof(resp.payload.message.topic_name) - 1);
    resp.payload.message.topic_name[sizeof(resp.payload.message.topic_name) - 1] = '\0';
    resp.payload.message.msg = *msg;
    return resp;
}

#endif //JRON_JRON_PROTOCOL_H