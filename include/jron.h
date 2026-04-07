//
// Created by jtwears on 3/25/26.
//

#ifndef JRON_SERVER_H
#define JRON_SERVER_H

#include <stddef.h>
//#include "broker.h"

// ============================================================================
// SERVER - Epoll loop, command parsing, per-client buffers
// ============================================================================

// Per-client state
typedef struct {
    int fd;
    char *read_buffer;         // For parsing incoming commands
    size_t read_pos;
    size_t read_capacity;

    char *write_buffer;        // For queueing outgoing messages to send
    size_t write_pos;
    size_t write_len;
    size_t write_capacity;
} client_t;

typedef struct {
    int listen_fd;
    int conn_fd;
    int epoll_fd;
    int client_count;
    int clients_capacity;
    struct epoll_event *events;
    //message_broker_t broker;

    client_t *clients;


    volatile int running;
} server_t;


// Lifecycle
int jron_init(server_t *server, const char *socket_path);
int jron_shutdown(server_t *server);

// Main loop
int jron_run(server_t *server);

// Event handlers
int jron_handle_new_connection(server_t *server);
int jron_handle_existing_clients(server_t *server, int file_descriptor);


#endif //JRON_SERVER_H