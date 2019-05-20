/* Copyright (c) 2019
 * Alexander Shirokov
 * Schneider Electric
 * See LICENSE for details. */
#ifdef __cplusplus
extern "C" {
#endif

#include "utils/base/link.h"
#include <assert.h>
#include <errno.h>
/* Инициализация WSA. Про очистку (WSACleanup) я не забыл, а просто
 * проигнорировал, т.к. у нас не продпологается несколько вызовов
 * WSAStartup/WSACleanup.
 * Функция не является потокобезопасной. Но это так же не является
 * проблемой, т.к. мы не предпологаем использование потоков */
static int wsa_init()
{
    static int wsa_init = 0;
    if(!wsa_init) {
        WSADATA wsa = {0};
        int err =  WSAStartup(MAKEWORD(2, 2), &wsa);
        if( err )
            return -err;

        wsa_init = 1;
    }
    return 0;
}

static int base_init(struct link * self, const char * ip, uint16_t port, uint16_t timeout)
{

    int err = wsa_init();

    if(err)
        return -err;

    struct sockaddr_in addr;
    memset(self, 0, sizeof(*self));
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip);
    if(addr.sin_addr.s_addr == INADDR_NONE) {
        return -WSAEINVAL;
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

    DWORD sec = self->timeout / 1000;
    DWORD msec = (self->timeout - (self->timeout / 1000) * 1000);
    DWORD timeout = sec * 1000 + msec;

    if(setsockopt(s, SOL_SOCKET,SO_RCVTIMEO, (const char *)&timeout, sizeof(timeout))) {
        goto error;
    }

    if(setsockopt(s, SOL_SOCKET,SO_SNDTIMEO, (const char *)&timeout, sizeof(timeout))) {
        goto error;
    }

    int len = sizeof(self->remote);

    /* Таймауты, установленные ранее, не работает для connect'а ->
     * Коннект может занимать непредсказуемое кол-во времени (обычно ~10-15 секунд, но
     * зависит от настроек в реестре)
     * Переходить на select и неблокирующие сокеты я пока не хочу. Причин
     * несколько:
     * - это актуально только для TCP. У нас основной режим работы - UDP
     * - у нас нет особых требований по скорости реакции системы. 5-10 секунд не
     * критичны
     * - это особая фишка винды. Работа в винде больше опциональная и требуется
     * для упрощения настройки с компов без линухи */
    if(connect(s,
               (struct sockaddr*)&self->remote,
               len) != 0)
        goto error;

    self->socket = s;
    return 0;

error:
    closesocket(s);
    return -errno;
}

void link_down(struct link * self)
{
    if(self->socket != TEKON_INVALID_SOCKET) {
        shutdown(self->socket, SB_BOTH);
        closesocket(self->socket);
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

    ssize_t result = send(self->socket, data, len, 0);

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

    ssize_t result = recv(self->socket, data, len, 0);

    if(result >= 0)
        return result;
    else
        return -errno;
}

#ifdef __cplusplus
}
#endif
