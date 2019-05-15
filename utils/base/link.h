/* Copyright (c) 2019
 * Alexander Shirokov
 * Schneider Electric
 * See LICENSE for details. */

#ifndef UTILS_BASE_LINK_H
#define UTILS_BASE_LINK_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#if defined(__unix__) || defined(__linux__)
// UNIX or LINUX
#include <sys/types.h>
#include <sys/socket.h>
#define TEKON_INVALID_SOCKET (-1)
typedef int socket_t;
#elif defined(_WIN32) || defined(WIN32)
// WINDOWS
#include <windows.h>
typedef SOCKET socket_t;
#define TEKON_INVALID_SOCKET (INVALID_SOCKET)
#endif


/* Тип связи */
enum link_type {LINK_UDP, LINK_TCP};

/* Нам требуется простая реализация сетевого обмена. Под простой я имею ввиду:
 * - синхронную
 * - без лишних наворотов, т.к. требуется сделать банальный запрос <-> ответ
 * - с поддержкой tcp/udp
 */
struct link {
    socket_t socket;
    struct sockaddr remote;
    enum link_type type;
    uint16_t timeout;
};

/* Выполнить инициализацию для работы по TCP.
 * В случае успеха вернет 0. Иначе - код ошибки */
int link_init_tcp(struct link * self, const char * ip, uint16_t port, uint16_t timeout);

/* Выполнить инициализацию для работы по UDP.
 * В случае успеха вернет 0. Иначе - код ошибки */
int link_init_udp(struct link * self, const char * ip, uint16_t port, uint16_t timeout);

/* Поднять линк.
 * В случае успеха вернет 0. Иначе - код ошибки */
int link_up(struct link * self);


/* Сбросить линк.
 * В случае успеха вернет 0. Иначе - код ошибки */
void link_down(struct link * self);

/* Отправить данные */
ssize_t link_send(struct link * self, const void * data, size_t len);

/* Получить данные */
ssize_t link_recv(struct link * self, void * data, size_t len);


#ifdef __cplusplus
}
#endif

#endif
