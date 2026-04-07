//
// Created by jtwears on 4/7/26.
//

#include "socket.h"
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>


int socket_create_and_bind(const char *socket_path) {
    const int socket_fd = socket(AF_UNIX, SOCK_SEQPACKET, 0);
    if (socket_fd < 0) {
        perror("socket");
        return EXIT_FAILURE;
    }

    struct sockaddr_un addr = {0};
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path) - 1);

    unlink(socket_path); // Remove existing socket file if it exists

    if (bind(socket_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(socket_fd);
        return EXIT_FAILURE;
    }

    return socket_fd;
}


int socket_listen(const int socket_fd) {
    const int res = listen(socket_fd, DEFAULT_MAX_CONNECTIONS);
    if (res < 0) {
        perror("listen");
        return EXIT_FAILURE;
    }
    return res;
}

int socket_accept(const int socket_fd) {
    const int client_fd = accept(socket_fd, NULL, NULL);
    if (client_fd < 0) {
        perror("accept");
        return EXIT_FAILURE;
    }
    return client_fd;
}

ssize_t socket_read(const int fd, void *buffer, const size_t count) {
    return read(fd, buffer, count);
}

ssize_t socket_write(const int fd, const void *buffer, const size_t count) {
    // check buffer size before writing
    return write(fd, buffer, count);
}

void socket_close(const int fd) {
    close(fd);
}

int socket_set_nonblocking(const int fd) {
    const int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) {
        perror("fcntl(F_GETFL)");
        return EXIT_FAILURE;
    }
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0) {
        perror("fcntl(F_SETFL)");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}