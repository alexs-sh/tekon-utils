/* Copyright (c) 2019
 * Alexander Shirokov
 * Schneider Electric
 * See LICENSE for details. */

#ifdef __cplusplus
extern "C" {
#endif

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <inttypes.h>
#include <getopt.h>

#include "tekon/tekon.h"
#include "utils/arch/arch.h"
#include "utils/base/base.h"

#define APP_NAME "tekon_arch"
#define APP_ERR  LOG_ERR  APP_NAME " : ERR"
#define APP_WARN LOG_WARN APP_NAME " : WARN"
#define APP_INFO LOG_INFO APP_NAME " : INFO"

struct app {

    struct netaddr netcfg;
    struct dtaddr dtcfg;
    struct link link;

    struct archive archive;

    struct devtime begin_at;
    struct devtime end_at;

    int tzoffset;
    int timeout;
    int use_tsc; /*time stamp converter*/
};

static void apply_noconn(struct rec * rec, void * data);
static void print(struct rec * self, void * data );

static void usage()
{
    printf("Usage: %s -a address -p parameters [-t timeout] [-v verbosity]\n\n", APP_NAME);
    printf("  -a    gateway's address in [type:ip:port@gateway] format.\n\n");
    printf("  -p    parameter for reading in [device:parameter:index:count:type] format.\n");
    printf("        index - start index\n");
    printf("        count - number of records to read. Should be less or equal %d\n", ARCHIVE_MAX_SIZE);
    printf("        type: \n");
    printf("            F - 32-bit float\n");
    printf("            U - 32-bit unsigned integer\n");
    printf("            H - 32-bit unsigned integer (HEX)\n");
    printf("            B - boolean\n");
    printf("            R - raw\n\n");
    printf("  -d    date/time addresses in [device:dateaddr:timeaddr] format.\n\n");
    printf("  -i    interval description in [type:depth:interval] format.\n");
    printf("        type:\n");
    printf("            m - months [12,48]\n");
    printf("            d - days\n");
    printf("            h - hours [384, 768, 1536]\n");
    printf("            i - interval\n\n");
    printf("  -t    response timeout in milliseconds\n\n");
    printf("  -v    set verbose:\n");
    printf("        0 - silent \n");
    printf("        1 - error\n");
    printf("        2 - warning \n");
    printf("        3 - info \n\n");
    printf("Example:\n");
    printf("  %s -a udp:10.0.0.3:51960@2 -p 3:0x8017:0:1536:F\n", APP_NAME);
    printf("  %s -a udp:10.0.0.3:51960@2 -p 3:0x801C:0:12:F -i m:12 -d 3:0xF017:0xF018\n", APP_NAME);
    printf("  %s -a udp:10.0.0.3:51960@2 -p 3:0x800D:0:1536:F -i h:1536 -d 3:0xF017:0xF018\n", APP_NAME);
    printf("  %s -a udp:10.0.0.3:51960@2 -p 4:0x8217:950:40:F -i i:1440:5 -d 3:0xF017:0xF018\n", APP_NAME);
}


static void init(struct app * self)
{
    assert(self);
    memset(self, 0, sizeof(*self));
    archive_init(&self->archive);
    self->tzoffset = time_tzoffset();
    self->timeout = 1000;
}

/* Запрос - ответ
 * 0 - в случае ошибки */
static int process_request(const struct message * request, struct message * response, struct link * link)
{
    assert(request);
    assert(response);
    assert(link);

    char in[512];
    char out[512];

    uint8_t nin = 0;
    static uint8_t nout = 0;
    nout = (nout + 1) % 16;

    int result = tekon_req_pack(out, sizeof(out), request, nout);

    if(result <= 0) {
        log_print(APP_ERR " : packing error\n");
        return 0;
    }

    result = link_send(link, out, result);
    if(result <= 0) {
        log_print(APP_ERR " : sending error %d\n", result);
        return 0;
    }

    result = link_recv(link, in, sizeof(in));
    if(result <= 0) {
        log_print(APP_ERR " : receiving error %d\n", result);
        return 0;
    }

    result = tekon_resp_unpack(in, result, response, request->type, &nin);

    if(result <= 0) {
        log_print(APP_ERR " : unpacking error\n");
        return 0;
    }

    return nin == nout && request->nelements == response->nelements;
}

/* Прочитать время из устройства */
static int read_time(struct tekon_date * date, struct tekon_time * time, const struct dtaddr * dtaddr, struct link * link)
{

    uint8_t size = 2;
    uint8_t devices[2] = {dtaddr->device, dtaddr->device};
    uint16_t addresses[2] = {dtaddr->date,dtaddr->time};
    uint16_t indexes[2] = {0};

    struct message request;
    struct message response;

    int result = tekon_req_1c(&request, dtaddr->gateway, devices, addresses, indexes, size);
    if(!result) {
        log_print(APP_ERR " : can't create request\n");
        return 0;
    }

    result = process_request(&request, &response, link) > 0 &&
             tekon_date_unpack(date, &response.payload.parameters[0].value, 4) &&
             tekon_time_unpack(time, &response.payload.parameters[1].value, 4);

    if(!result)
        log_print(APP_ERR " : date/time reading failed\n");

    return result;
}

/* Прочитать часть архива (<= 40 записей)
 * 0 - в случае ошибки */
static int read_chunk(struct archive * archive, size_t pos, size_t size, struct link * link)
{
    assert(size <= TEKON_PROTO_PLIST_SIZE);


    uint8_t gateway = archive->address.gateway;
    uint8_t devices[TEKON_PROTO_PLIST_SIZE];
    uint16_t addresses[TEKON_PROTO_PLIST_SIZE];
    uint16_t indexes[TEKON_PROTO_PLIST_SIZE];

    struct message request;
    struct message response;

    size_t i;

    for(i = 0; i < size; i++) {
        devices[i] =archive->address.device;
        addresses[i] = archive->address.address;
        indexes[i] = archive_get(archive, pos + i)->index;
    }

    int result = tekon_req_1c(&request, gateway, devices, addresses, indexes, size);

    if(!result) {
        log_print(APP_ERR " : can't create request\n");
        return 0;
    }

    result = process_request(&request, &response, link);

    const struct tekon_parameter * param = response.payload.parameters;
    for(i = 0; i < size; i++, param++) {

        const uint32_t * value = NULL;
        enum quality qual = Q_NOCONN;
        size_t len = 0;
        if(result > 0 ) {
            value = &param->value;
            qual = param->qual == 0 ? Q_OK : Q_INVALID;
            len = 4;
        }

        rec_update(archive_get(archive, pos + i), qual, value, len);

    }
    return result > 0;
}

/* Прочитать весь архив
 * 0 - в случае ошибки */
static int read_archive(struct app * app)
{
    const size_t lim = archive_size(&app->archive);
    size_t pos = 0;

    while(pos < lim) {
        const size_t diff = lim - pos;
        const size_t chunk = diff == 1 ? 1 :
                             diff > TEKON_PROTO_PLIST_SIZE ? TEKON_PROTO_PLIST_SIZE :
                             diff;


        /* Если порция данных была прочитана с ошибкой, то нет смысла читать
         * остальные. Просто стивим всем оставшимся ошибку связи. Чтобы
         * сделать это с минимальным трудом, кладем коннект и пытаемся
         * вычитать данные на отключенном линке. */
        if(!read_chunk(&app->archive, pos, chunk, &app->link)) {
            log_print(APP_ERR " : archive reading failed at %zd:%zd:%zd\n", pos, chunk, lim);
            return 0;
        }
        pos += chunk;
    }
    return 1;
}

/* Прочитать данные с утсройства.
 * В зависимости от куонифгурации читает либо только архив (значения + индексы),
 * либо архив + время начала/окончания из Тэкона. В дальнейшем это время можно
 * использовать для перевода индексов в метки времени
 */
static int read(struct app * app)
{
    /* создать подключение */
    if(app->netcfg.type == LINK_TCP)
        link_init_tcp(&app->link, app->netcfg.ip, app->netcfg.port, app->timeout);
    else if(app->netcfg.type == LINK_UDP)
        link_init_udp(&app->link, app->netcfg.ip, app->netcfg.port, app->timeout);

    /* Установить подключение и флаг нет связи */
    int result = link_up(&app->link);

    archive_foreach(&app->archive, apply_noconn, NULL);

    if(result != 0) {
        log_print(APP_ERR " : connecting error %d\n", result);
        return 0;
    }

    /* Прочитать время с утсройства */
    if(app->use_tsc) {
        if(!read_time(&app->begin_at.date, &app->begin_at.time, &app->dtcfg, &app->link)) {
            link_down(&app->link);
            return 0;
        }
    } else {
        log_print(APP_WARN " : timestamp converter disabled\n");
    }


    /* Прочитать архив */
    if(!read_archive(app)) {
        link_down(&app->link);
        return 0;
    }

    /* Прочитать время с утсройства */
    if(app->use_tsc) {
        if(!read_time(&app->end_at.date, &app->end_at.time, &app->dtcfg, &app->link)) {
            link_down(&app->link);
            return 0;
        }

    }

    /* Закрыть коннект */
    link_down(&app->link);
    return 1;
}

/* Прочитать параметры командной строки
 * 0 - в случае ошибки */
static int read_args(struct app * app, int argc, char * const argv[])
{
    assert(app);

    if(argc < 4)
        return 0;

    int opt;
    uint8_t gateway = 0;


    while ((opt = getopt(argc, argv, "t:a:p:i:d:v:")) != -1) {
        switch (opt) {
        case 't': {
            long input  = atol(optarg);
            if(input <= 0 || input > 60000) {
                printf("invalid timeout %s\n\n", optarg);
                return 0;
            } else {
                app->timeout = (size_t)input;
            }
        }
        break;
        case 'p':
            if(gateway == 0) {
                printf("enter address before parameters list\n\n");
                return 0;
            }

            if(!archaddr_from_string(&app->archive.address, optarg)) {
                printf("invalid parameter address %s\n\n", optarg);
                return 0;
            }
            break;
        case 'a':
            if(!netaddr_from_string(&app->netcfg, optarg)) {
                printf("invalid network address %s\n\n", optarg);
                return 0;
            }
            gateway = app->netcfg.gateway;
            break;
        case 'd':
            if(!dtaddr_from_string(&app->dtcfg, optarg)) {
                printf("invalid date/time address %s\n\n", optarg);
                return 0;
            }
            gateway = app->netcfg.gateway;
            break;
        case 'i':
            if(!intcfg_from_string(&app->archive.interval, optarg)) {
                printf("interval address is invalid %s\n\n", optarg);
                return 0;
            }
            break;
        case 'v':
            log_setlevel(atoi(optarg));
            break;
        default: /* '?' */
            printf("invalid argument %c\n\n", opt);
            return 0;
        }
    }

    /* Дописать инфу */
    app->archive.address.gateway = gateway;
    app->dtcfg.gateway = gateway;
    app->use_tsc = app->dtcfg.gateway != TEKON_INVALID_DEV_ADDR &&
                   app->dtcfg.device != TEKON_INVALID_DEV_ADDR &&
                   app->archive.interval.type != 0;

    /* Адрес не задан */
    if(app->netcfg.port == 0) {
        printf("please enter valid gateway address\n\n");
        return 0;
    }

    /* Архив не задан */
    if(app->archive.address.device == 0) {
        printf("please enter valid archive address\n\n");
        return 0;
    }

    /* Введены неподдерживаемые типы */
    enum tekon_parameter_type type = app->archive.address.type;
    if(!(type == TEKON_PARAM_F32 ||
            type == TEKON_PARAM_U32 ||
            type == TEKON_PARAM_HEX ||
            type == TEKON_PARAM_BOOL ||
            type == TEKON_PARAM_RAW)) {
        printf("unsupported type\n\n");
        return 0;
    }

    /* Введены недопустимые параметры интервала/параметра */
    const char archtype = app->archive.interval.type;
    const uint16_t size = app->archive.address.count;
    const uint16_t start = app->archive.address.index;
    const uint16_t limit = archtype == 'd' ? 366 : app->archive.interval.depth;

    if(size == 0) {
        printf("please enter parameters to read\n\n");
        return 0;
    }

    if(size > ARCHIVE_MAX_SIZE) {
        printf("archive overflow. Can't read more than %d values\n\n", ARCHIVE_MAX_SIZE);
        return 0;
    }

    if(limit !=0 &&
            start + size > limit) {
        printf("please enter valid parameter and interval\n");
        return 0;
    }

    return 1;
}

/* Проинициализировать архив пустыми значениями
 * 0 - в случае ошибки */
static int fill_acrhive(struct archive * archive)
{
    assert(archive);

    /* Заполнить архив */
    size_t i;
    for(i = 0; i < archive->address.count; i++) {
        struct rec rec;
        rec_init(&rec, archive->address.index + i);
        if(!archive_add(archive, &rec)) {
            return 0;
        }
    }
    /* Параметры не заданы */
    if(archive_size(archive) == 0) {
        return 0;
    }

    return 1;
}

/* Установить качестве "Нет связи" и сбросить все значения в 0 */
static void apply_noconn(struct rec * rec, void * data)
{
    rec->qual = Q_NOCONN;
    rec->value.u32 = 0;
}

/* Печать записи из таблицы измерений */
static void print(struct rec * self, void * data )
{
    /* Общий формат вывода: адрес тип значение качество время сдвиг [индекс] */
    assert(self);
    assert(data);

    char buffer[512] = {0};
    size_t remain = sizeof(buffer);
    char * ptr = buffer;
    const struct app * app = data;
    const struct paraddr * addr = &app->archive.address;

    /* Добавить адрес шлюза */
    int result = snprintf(ptr, remain, "%"PRIu8":",addr->gateway);
    assert(result > 0);
    ptr += result;
    remain -= result;

    /* Добавить адрес устройства */
    result = snprintf(ptr, remain, "%"PRIu8":",addr->device);
    assert(result > 0);
    ptr += result;
    remain -= result;

    /* Добавить адрес параметра */
    result = addr->hex ?
             snprintf(ptr, remain, "0x%x:", addr->address) :
             snprintf(ptr, remain, "%"PRIu16":", addr->address);
    assert(result > 0);
    ptr += result;
    remain -= result;

    /* Добавить индекс параметра */
    result = snprintf(ptr, remain, "%"PRIu16" ", self->index);
    assert(result > 0);
    ptr += result;
    remain -= result;

    /* Добавить тип и значение */
    switch(app->archive.address.type) {
    case TEKON_PARAM_RAW:
        result = snprintf(ptr, remain, "R 0x%02x%02x%02x%02x ",self->value.byte[0], self->value.byte[1], self->value.byte[2], self->value.byte[3]);
        break;
    case TEKON_PARAM_U32:
        result = snprintf(ptr, remain, "U %"PRIu32" ",self->value.u32);
        break;
    case TEKON_PARAM_F32:
        result = snprintf(ptr, remain, "F %f ",self->value.f32);
        break;
    case TEKON_PARAM_BOOL:
        result = snprintf(ptr, remain, "B %s ",self->value.u32 > 0 ? "TRUE" : "FALSE");
        break;
    case TEKON_PARAM_HEX:
        result = snprintf(ptr, remain, "H 0x%x ",self->value.u32);
        break;
    case TEKON_PARAM_TIME:
    case TEKON_PARAM_DATE:
        break;

    }
    assert(result > 0);
    ptr += result;
    remain -= result;

    /* Добавить качество */
    switch(self->qual) {
    case Q_INVALID:
        result = snprintf(ptr, remain, "INV ");
        break;
    case Q_NOCONN:
        result = snprintf(ptr, remain, "COM ");
        break;
    case Q_UNK:
        result = snprintf(ptr, remain, "UNK ");
        break;
    case Q_OK:
        result = snprintf(ptr, remain, "OK ");
        break;
    }
    assert(result > 0);
    ptr += result;
    remain -= result;

    /* Добавить метку времени */
    result = snprintf(ptr, remain, "%"PRIi64" ", self->timestamp);
    assert(result>0);
    ptr += result;
    remain -= result;

    /* Добавить сдвиг часового пояса */
    result = snprintf(ptr, remain, "%"PRIi32" ", app->tzoffset);
    assert(result>0);

    /* Вывести */
    printf("%s\n", buffer);
}

static void sigint(int sig)
{
    log_print(APP_INFO " : stop\n");
}

int main(int argc, char * argv[])
{

    struct app app;

    init(&app);

    signal(SIGINT, sigint);

    /* прочитать аргменты командной строки */
    if(!read_args(&app, argc, argv)) {
        usage();
        return 1;
    }

    /* подготовить архив к работе */
    if(!fill_acrhive(&app.archive))
        return 1;

    /* Прочитать данные из утсройства */
    int result = read(&app);

    /* Перевести индексы в метки времени */
    archive_index_to_utc(&app.archive, &app.begin_at, &app.end_at);

    /* Вывести результат */
    archive_foreach(&app.archive, print, &app);
    return result == 0;
}

#ifdef __cplusplus
}
#endif
