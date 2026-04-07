//
// Created by jtwears on 3/26/26.
//

#include "jron.h"
#include "socket.h"
#include <stdlib.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/socket.h>


int jron_init(server_t *server, const char *socket_path) {
    if (!server || !socket_path) return EXIT_FAILURE;

    server->conn_fd = socket_create_and_bind(socket_path);
    if (server->conn_fd < 0) return EXIT_FAILURE;

    server->listen_fd = socket_listen(server->conn_fd);

    if (server->listen_fd < 0) return EXIT_FAILURE;

    // if (broker_init(&server->broker) < 0) {
    //     socket_close(server->socket_fd);
    //     return EXIT_FAILURE;
    // }

    server->clients = NULL;
    server->client_count = 0;
    server->clients_capacity = 0;
    server->running = 1;
    return EXIT_SUCCESS;
}

int jron_shutdown(server_t *server) {
    if (!server) return EXIT_FAILURE;

    server->running = 0;
    server->client_count = 0;
    server->clients_capacity = 0;
    free(server->clients); // is this a bug? and a segfault waiting to happen?
    server->clients = NULL;
    socket_close(server->conn_fd);
    return EXIT_SUCCESS;
}

int jron_run(server_t *server) {
    if (!server) return EXIT_FAILURE;

    jron_init(server, DEFAULT_SOCKET_NAME);
    server->epoll_fd = epoll_create1(0);
    if (server->epoll_fd < 0) {
        perror("epoll_create1");
        jron_shutdown(server);
        return EXIT_FAILURE;
    }

    server->events->events = EPOLLIN;
    server->events->data.fd = server->listen_fd;
    if (epoll_ctl(server->epoll_fd, EPOLL_CTL_ADD, server->listen_fd, &server->events[0]) < 0) {
        perror("epoll_ctl: listen_fd");
        close(server->epoll_fd);
        jron_shutdown(server);
        return EXIT_FAILURE;
    }

    while (server->running) {
        int nfds = epoll_wait(server->epoll_fd, server->events, server->client_count + 1, -1);
        if (nfds == -1) {
            perror("epoll_wait");
            return EXIT_FAILURE;
        }

        // handle the events
        for (int i = 0; i < nfds; i++) {
            // handle new client connections
            if (server->events[i].data.fd == server->listen_fd) {
                const int ok = jron_handle_new_connection(server);
                if (ok == -1) {
                    perror("server_handle_new_connection");
                    jron_shutdown(server);
                    return EXIT_FAILURE;
                }
            } else {
                jron_handle_existing_clients(server, server->events[i].data.fd);
            }
        }
    }
    return EXIT_SUCCESS;
}

int jron_handle_new_connection(server_t *server) {
    server->conn_fd = accept(server->listen_fd, NULL, NULL);
    if (server->conn_fd == -1) {
        perror("accept");
        return -1;
    }

    socket_set_nonblocking(server->conn_fd);
    server->events->events = EPOLLIN | EPOLLET;
    server->events->data.fd = server->conn_fd;
    if (epoll_ctl(server->epoll_fd, EPOLL_CTL_ADD, server->conn_fd, &server->events[0]) == -1) {
        perror("epoll_ctl: conn_fd");
        return -1;
    }

    return 0;
}

int jron_handle_existing_clients(server_t *server, int file_descriptor) {
    printf("handle_existing_clients\n");
    printf("fd: %d\n", file_descriptor);
    // handle actual broker logic here, for now just read and print
    // In a real implementation, this is where we would parse commands from the client,
    // interact with the message broker, and queue responses to send back.
    return EXIT_SUCCESS;
}