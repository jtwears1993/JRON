//
// Created by jtwears on 4/7/26.
//

#ifndef JRON_SOCKET_H
#define JRON_SOCKET_H
#include <stddef.h>
#include <stdio.h>

#define DEFAULT_MAX_CONNECTIONS 10
#define DEFAULT_SOCKET_NAME "/tmp/jron.sock"

/*
 * The socket module will handle all low-level networking operations, including:
 * 1. create socket
 * 2. bind
 * 3. listen
 * 4. main accept loop
 * 5. client read/write operations
 * 6. client disconnect handling
 * 7. close/ cleanup on shutdown
 */

int socket_create_and_bind(const char *socket_path);
int socket_listen(int socket_fd);
int socket_accept(int socket_fd);
ssize_t socket_read(int fd, void *buffer, size_t count);
ssize_t socket_write(int fd, const void *buffer, size_t count);
void socket_close(int fd);
int socket_set_nonblocking(int fd);


#endif //JRON_SOCKET_H