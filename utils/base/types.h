/* Copyright (c) 2019
 * Alexander Shirokov
 * Schneider Electric
 * See LICENSE for details. */

#ifndef UTILS_BASE_TYPES_H
#define UTILS_BASE_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include "utils/base/link.h"
#include "tekon/tekon.h"
/* Типы, встречающиеся в различных утилитах */

/* Качество параметра */
enum quality { Q_UNK,
               Q_OK,
               Q_NOCONN,
               Q_INVALID
             };


/* Сетевой адрес для подключения к Тэкон
 * udp:192.168.1.3:51960@2 */
struct netaddr {
    char ip[32];
    uint16_t port;
    enum link_type type;
    uint8_t gateway;
};

/* Адрес и информация о параметре Тэкона
 * 3:12345:0:F - параметр
 * 3:12345:0:1440:F - архив */
struct paraddr {
    uint8_t gateway;
    uint8_t device;
    uint16_t address;
    uint16_t index;
    enum tekon_parameter_type type;
    char hex;
    uint16_t count;
};

/* Адрес времени в Теконе
 * 3:0xF017:0xF018 */
struct dtaddr {
    uint8_t gateway;
    uint8_t device;
    uint16_t date;
    uint16_t time;
};

/* Описание интервала */
struct intcfg {
    uint16_t depth;
    uint8_t interval;
    char type;
};

int netaddr_from_string(struct netaddr * self, const char * str);
int paraddr_from_string(struct paraddr * self, const char * str);
int archaddr_from_string(struct paraddr * self, const char * str);
int dtaddr_from_string(struct dtaddr * self, const char * str);
int intcfg_from_string(struct intcfg * self, const char * str);


#ifdef __cplusplus
}
#endif

#endif
