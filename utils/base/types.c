/* Copyright (c) 2019
 * Alexander Shirokov
 * Schneider Electric
 * See LICENSE for details. */

#ifdef __cplusplus
extern "C" {
#endif
#include "utils/base/types.h"

#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "utils/base/tstamp.h"
#include "utils/base/string.h"

#if defined(__unix__) || defined(__linux__)
#include <arpa/inet.h>  //inet_addr
#endif


static int validate_ip(const char * ip)
{
    assert(ip);
    return inet_addr(ip) != INADDR_NONE;
}

static int intcfg_is_valid(const struct intcfg * self)
{
    switch(self->type) {
    case 'm':
        return self->depth == 12 ||
               self->depth == 48;
    case 'd':
        return self->depth == 0 ||
               self->depth == 365 ||
               self->depth == 366;
    case 'h':
        return self->depth == 16*24 ||
               self->depth == 32 * 24 ||
               self->depth == 64 * 24;
    case 'i':
        return self->depth > 0 &&
               self->depth < TIMESTAMP_MAX_SEQ_SIZE &&
               self->interval > 0 &&
               self->interval < 100;
    }
    return 0;
}

int netaddr_from_string(struct netaddr * self, const char * str)
{
    //udp:192.168.1.10:51960@1
    assert(self);
    if(string_is_term(str))
        return 0;

    struct netaddr result;
    const char * ptr = string_trim(str);
    const size_t typelen = 3;
    const size_t iplen = sizeof(result.ip) - 1;
    const size_t portlen = 5;
    const size_t gwlen = 4;

    char buffer[128] = {0};
    size_t cnt = 0;

    memset(&result, 0, sizeof(result));

    // прочитать тип адреса
    for(cnt = 0; cnt < typelen && !string_is_term(ptr) && *ptr != ':'; cnt++, ptr++) {
        buffer[cnt] = *ptr;
    }

    buffer[cnt] = '\0';

    if(cnt != typelen)
        return 0;

    if(strncmp(buffer, "udp", typelen) == 0)
        result.type = LINK_UDP;
    else if(strncmp(buffer, "tcp", typelen) == 0)
        result.type = LINK_TCP;
    else
        return 0;

    // прочитать IP
    ptr = string_next(ptr);
    for(cnt = 0; cnt < iplen && !string_is_term(ptr) && *ptr != ':'; cnt++, ptr++) {
        buffer[cnt] = *ptr;
    }

    buffer[cnt] = '\0';

    if(!validate_ip(buffer))
        return 0;

    strncpy(result.ip, buffer, cnt);

    // прочитать порт
    ptr = string_next(ptr);
    for(cnt = 0; cnt < portlen && !string_is_term(ptr) && *ptr != '@'; cnt++, ptr++) {
        buffer[cnt] = *ptr;
    }

    buffer[cnt] = '\0';

    long port = atol(buffer);
    if(port <= 0 || port > 65535)
        return 0;

    result.port = port;

    // прочитать адрес шлюза
    ptr = string_next(ptr);
    for(cnt = 0; cnt < gwlen && !string_is_term(ptr); cnt++, ptr++) {
        buffer[cnt] = *ptr;
    }

    buffer[cnt] = '\0';
    long gateway = atol(buffer);
    if(gateway <= 0 || gateway > 255)
        return 0;

    result.gateway = gateway;
    memcpy(self, &result, sizeof(result));
    return 1;
}

int paraddr_from_string(struct paraddr * self, const char * str)
{
    //1:0x2000:0:F
    //1:123:0:I
    assert(self);
    if(string_is_term(str))
        return 0;

    struct paraddr result;
    const char * ptr = string_trim(str);
    const size_t devlen = 3;
    const size_t parlen = 6;
    const size_t indlen = 5;
    const size_t typelen = 1;

    char buffer[128] = {0};
    size_t cnt = 0;

    memset(&result, 0, sizeof(result));

    // прочитать адрес устройства
    for(cnt = 0; cnt < devlen && !string_is_term(ptr) && *ptr != ':'; cnt++, ptr++) {
        buffer[cnt] = *ptr;
    }

    buffer[cnt] = '\0';

    long device = atol(buffer);
    if(device <= 0 || device > 255)
        return 0;

    result.device = device;

    // прочитать № параметра. Адрес может быть как 10-чным, так и 16-ричным.
    ptr = string_next(ptr);
    for(cnt = 0; cnt < parlen && !string_is_term(ptr) && *ptr != ':'; cnt++, ptr++) {
        buffer[cnt] = *ptr;
    }

    buffer[cnt] = '\0';
    char is_hex = cnt > 2 && buffer[0] == '0' && buffer[1]=='x';
    long parameter = is_hex ? strtol(buffer, NULL, 16) : strtol(buffer, NULL, 10);
    if(parameter <= 0)
        return 0;

    result.address = parameter;
    result.hex = is_hex;

    // прочитать индекс
    ptr = string_next(ptr);
    for(cnt = 0; cnt < indlen && !string_is_term(ptr) && *ptr != ':'; cnt++, ptr++) {
        buffer[cnt] = *ptr;
    }

    buffer[cnt] = '\0';

    long index = atol(buffer);
    if(index < 0 || index > 65535)
        return 0;
    result.index = index;

    // прочитать тип
    ptr = string_next(ptr);
    for(cnt = 0; cnt < typelen && !string_is_term(ptr); cnt++, ptr++) {
        buffer[cnt] = *ptr;
    }

    buffer[cnt] = '\0';

    switch(tolower(buffer[0])) {
    case 'r':
        result.type = TEKON_PARAM_RAW;
        break;
    case 'b':
        result.type = TEKON_PARAM_BOOL;
        break;
    case 'u':
        result.type = TEKON_PARAM_U32;
        break;
    case 'h':
        result.type = TEKON_PARAM_HEX;
        break;
    case 'f':
        result.type = TEKON_PARAM_F32;
        break;
    case 't':
        result.type = TEKON_PARAM_TIME;
        break;
    case 'd':
        result.type = TEKON_PARAM_DATE;
        break;
    default:
        return 0;
    }

    memcpy(self, &result, sizeof(result));
    return 1;
}

int archaddr_from_string(struct paraddr * self, const char * str)
{

    //3:12345:0:1440:F
    //3:0x801C:0:1440:F
    assert(self);
    if(string_is_term(str))
        return 0;

    struct paraddr result;
    const char * ptr = string_trim(str);
    const size_t devlen = 3;
    const size_t parlen = 6;
    const size_t indlen = 5;
    const size_t typelen = 1;

    char buffer[128] = {0};
    size_t cnt = 0;

    memset(&result, 0, sizeof(result));

    // прочитать адрес устройства
    for(cnt = 0; cnt < devlen && !string_is_term(ptr) && *ptr != ':'; cnt++, ptr++) {
        buffer[cnt] = *ptr;
    }

    buffer[cnt] = '\0';

    long device = atol(buffer);
    if(device <= 0 || device > 255)
        return 0;

    result.device = device;

    // прочитать № параметра. Адрес может быть как 10-чным, так и 16-ричным.
    ptr = string_next(ptr);
    for(cnt = 0; cnt < parlen && !string_is_term(ptr) && *ptr != ':'; cnt++, ptr++) {
        buffer[cnt] = *ptr;
    }

    buffer[cnt] = '\0';
    char is_hex = cnt > 2 && buffer[0] == '0' && buffer[1]=='x';
    long parameter = is_hex ? strtol(buffer, NULL, 16) : strtol(buffer, NULL, 10);
    if(parameter <= 0)
        return 0;

    result.address = parameter;
    result.hex = is_hex;

    // прочитать начальный индекс
    ptr = string_next(ptr);
    for(cnt = 0; cnt < indlen && !string_is_term(ptr) && *ptr != ':'; cnt++, ptr++) {
        buffer[cnt] = *ptr;
    }

    buffer[cnt] = '\0';

    long index = atol(buffer);
    if(index < 0 || index > 65535)
        return 0;
    result.index = index;

    // прочитать конечный индекс
    ptr = string_next(ptr);
    for(cnt = 0; cnt < indlen && !string_is_term(ptr) && *ptr != ':'; cnt++, ptr++) {
        buffer[cnt] = *ptr;
    }

    buffer[cnt] = '\0';

    long count = atol(buffer);
    if(count < 0 || count > 65535)
        return 0;
    result.count = count;

    // прочитать тип
    ptr = string_next(ptr);
    for(cnt = 0; cnt < typelen && !string_is_term(ptr); cnt++, ptr++) {
        buffer[cnt] = *ptr;
    }

    buffer[cnt] = '\0';

    switch(tolower(buffer[0])) {
    case 'r':
        result.type = TEKON_PARAM_RAW;
        break;
    case 'b':
        result.type = TEKON_PARAM_BOOL;
        break;
    case 'u':
        result.type = TEKON_PARAM_U32;
        break;
    case 'h':
        result.type = TEKON_PARAM_HEX;
        break;
    case 'f':
        result.type = TEKON_PARAM_F32;
        break;
    case 't':
        result.type = TEKON_PARAM_TIME;
        break;
    case 'd':
        result.type = TEKON_PARAM_DATE;
        break;
    default:
        return 0;
    }

    memcpy(self, &result, sizeof(result));
    return 1;

}

int dtaddr_from_string(struct dtaddr * self, const char * str)
{
    //3:0xF017:0xF018
    assert(self);
    if(string_is_term(str))
        return 0;

    struct dtaddr result;
    const char * ptr = string_trim(str);
    const size_t devlen = 3;
    const size_t parlen = 6;

    char buffer[128] = {0};
    size_t cnt = 0;

    memset(&result, 0, sizeof(result));

    // прочитать адрес устройства
    for(cnt = 0; cnt < devlen && !string_is_term(ptr) && *ptr != ':'; cnt++, ptr++) {
        buffer[cnt] = *ptr;
    }

    buffer[cnt] = '\0';

    long device = atol(buffer);
    if(device <= 0 || device > 255)
        return 0;

    result.device = device;

    // прочитать адрес даты. Может быть как 10-чным, так и 16-ричным.
    ptr = string_next(ptr);
    for(cnt = 0; cnt < parlen && !string_is_term(ptr) && *ptr != ':'; cnt++, ptr++) {
        buffer[cnt] = *ptr;
    }

    buffer[cnt] = '\0';
    char is_hex = cnt > 2 && buffer[0] == '0' && buffer[1]=='x';
    long parameter = is_hex ? strtol(buffer, NULL, 16) : strtol(buffer, NULL, 10);
    if(parameter <= 0)
        return 0;

    result.date = parameter;

    // прочитать адрес времени. Может быть как 10-чным, так и 16-ричным.
    ptr = string_next(ptr);
    for(cnt = 0; cnt < parlen && !string_is_term(ptr) && *ptr != ':'; cnt++, ptr++) {
        buffer[cnt] = *ptr;
    }

    buffer[cnt] = '\0';
    is_hex = cnt > 2 && buffer[0] == '0' && buffer[1]=='x';
    parameter = is_hex ? strtol(buffer, NULL, 16) : strtol(buffer, NULL, 10);
    if(parameter <= 0)
        return 0;

    result.time = parameter;

    memcpy(self, &result, sizeof(result));
    return 1;
}

int intcfg_from_string(struct intcfg * self, const char * str)
{
    //Y:12
    //I:1440:5
    assert(self);
    if(string_is_term(str))
        return 0;


    struct intcfg result;
    const char * ptr = string_trim(str);
    const size_t depthlen = 4;
    const size_t intlen = 2;

    char buffer[128] = {0};
    size_t cnt = 0;

    memset(&result, 0, sizeof(result));

    char type = tolower(*ptr);
    switch(type) {
    case 'm':
        break;
    case 'd':
        break;
    case 'h':
        break;
    case 'i':
        break;
    default:
        return 0;
    }
    result.type = type;

    // прочитать глубину архива
    ptr = string_next(ptr);
    ptr = string_next(ptr);
    for(cnt = 0; cnt < depthlen && !string_is_term(ptr) && *ptr != ':'; cnt++, ptr++) {
        buffer[cnt] = *ptr;
    }

    buffer[cnt] = '\0';

    result.depth = atoi(buffer);

    // Прочитать интервал архива
    ptr = string_next(ptr);
    for(cnt = 0; cnt < intlen && !string_is_term(ptr) && *ptr != ':'; cnt++, ptr++) {
        buffer[cnt] = *ptr;
    }

    buffer[cnt] = '\0';
    result.interval = atoi(buffer);

    if(!intcfg_is_valid(&result))
        return 0;

    memcpy(self, &result, sizeof(result));
    return 1;
}

#ifdef __cplusplus
}
#endif




