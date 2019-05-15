/* Copyright (c) 2019
 * Alexander Shirokov
 * Schneider Electric
 * See LICENSE for details. */

#ifdef __cplusplus
extern "C" {
#endif

#include "utils/base/link.h"
#include <assert.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>

static int base_init(struct link * self, const char * ip, uint16_t port, uint16_t timeout)
{
    struct sockaddr_in addr;
    memset(self, 0, sizeof(*self));
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip);
    if(addr.sin_addr.s_addr == INADDR_NONE) {
        return -EINVAL;
    }

    self->socket = TEKON_INVALID_SOCKET;
    memcpy(&self->remote, &addr, sizeof(self->remote));
    self->timeout = timeout;
    return 0;

}

int link_init_tcp(struct link * self, const char * ip, uint16_t port, uint16_t timeout)
{
    assert(self);
    assert(ip);
    assert(port);

    int err = base_init(self, ip, port, timeout);

    if(!err)
        self->type = LINK_TCP;

    return err;
}

int link_init_udp(struct link * self, const char * ip, uint16_t port, uint16_t timeout)
{
    assert(self);
    assert(ip);
    assert(port);

    int err = base_init(self, ip, port, timeout);

    if(!err)
        self->type = LINK_UDP;

    return err;
}

int link_up(struct link * self)
{
    assert(self);

    if(self->socket != TEKON_INVALID_SOCKET)
        return -EISCONN;

    socket_t s = socket(AF_INET,
                        self->type == LINK_UDP ? SOCK_DGRAM : SOCK_STREAM,
                        0);

    if (s == TEKON_INVALID_SOCKET)
        return -errno;

    struct timeval ts = {
        .tv_sec = self->timeout / 1000,
        .tv_usec = (self->timeout - (self->timeout / 1000) * 1000) * 1000
    };

    if(setsockopt(s, SOL_SOCKET,SO_RCVTIMEO, &ts, sizeof(ts))) {
        goto error;
    }

    if(setsockopt(s, SOL_SOCKET,SO_SNDTIMEO, &ts, sizeof(ts))) {
        goto error;
    }

    socklen_t len = sizeof(self->remote);

    if(connect(s,
               (struct sockaddr*)&self->remote,
               len) != 0)
        goto error;

    self->socket = s;
    return 0;

error:
    close(s);
    return -errno;
}

void link_down(struct link * self)
{
    if(self->socket != TEKON_INVALID_SOCKET) {
        shutdown(self->socket, SHUT_RDWR);
        close(self->socket);
        self->socket = TEKON_INVALID_SOCKET;
    }
}

ssize_t link_send(struct link * self, const void * data, size_t len)
{
    assert(self);
    assert(data);
    assert(len);
    if(self->socket == TEKON_INVALID_SOCKET)
        return -EBADF;

    ssize_t result = send(self->socket, data, len, MSG_NOSIGNAL);

    if(result >= 0)
        return result;
    else
        return -errno;
}

ssize_t link_recv(struct link * self, void * data, size_t len)
{
    assert(self);
    assert(data);
    assert(len);

    if(self->socket == TEKON_INVALID_SOCKET)
        return -EBADF;

    ssize_t result = recv(self->socket, data, len, MSG_NOSIGNAL);

    if(result >= 0)
        return result;
    else
        return -errno;
}

#ifdef __cplusplus
}
#endif
