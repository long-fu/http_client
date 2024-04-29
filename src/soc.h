#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
    int socket_create(char *ethx, char *src_ip, int src_port, const char *dest_ip, int dest_port);
    ssize_t socket_send(int __fd, const void *__buf, size_t __n, int __flags);
    ssize_t socket_recv(int __fd, void *__buf, size_t __n, int __flags);
    int socket_destroy(int sock);
#ifdef __cplusplus
}
#endif /* __cplusplus */