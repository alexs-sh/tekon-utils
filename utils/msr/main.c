/* Copyright (c) 2019
 * Alexander Shirokov
 * Schneider Electric
 * See LICENSE for details. */

#ifdef __cplusplus
extern "C" {
#endif

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <inttypes.h>

#include "utils/base/base.h"
#include "utils/msr/msr.h"
#include "tekon/tekon.h"

#define APP_NAME "tekon_msr"
#define APP_ERR  LOG_ERR  APP_NAME " : ERR"
#define APP_WARN LOG_WARN APP_NAME " : WARN"


struct app {
    struct netaddr netcfg;
    struct msr_table table;
    struct link link;
    int tzoffset;
    int timeout;
};

/* Установить записи качество Q_NOCONN и обновить метку времени */
static void apply_noconn(struct msr * msr, void * data)
{
    assert(data);
    int64_t *tstamp = data;
    msr->qual = Q_NOCONN;
    msr->timestamp = *tstamp;

}

static void init(struct app * self)
{
    assert(self);
    memset(self, 0, sizeof(*self));
    msr_table_init(&self->table);
    self->tzoffset = time_tzoffset();
    self->timeout = 1000;
}

static void usage()
{
    printf("Usage: %s -a address -p parameters [-t timeout] [-v verbosity]\n\n", APP_NAME);
    printf("  -a    gateway's address in [type:ip:port@gateway] format.\n\n");
    printf("  -p    list of parameters in device:parameter:index:type format.\n");
    printf("        type: \n");
    printf("            F - 32-bit float\n");
    printf("            U - 32-bit unsigned integer\n");
    printf("            H - 32-bit unsigned integer (HEX)\n");
    printf("            B - boolean\n\n");
    printf("            R - raw\n\n");
    printf("            D - date\n");
    printf("            T - time\n");
    printf("  -t    response timeout in milliseconds.\n\n");
    printf("  -v    set verbose:\n");
    printf("        0 - silent \n");
    printf("        1 - error\n");
    printf("        2 - warning \n");
    printf("        3 - info \n\n");
    printf("Example:\n");
    printf("  %s -a udp:10.0.0.3:51960@2 -p '3:0xF001:0:R 3:0x8003:0:F 3:0xF017:0:D 3:0xF018:0:T'\n", APP_NAME);
}

/* Запрос-ответ
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

/* Прочитать набор параметров
 * 0 - в случае ошибки */
static int read_multiple(struct msr * msr, size_t size, struct link * link)
{
    assert(size <= TEKON_PROTO_PLIST_SIZE);

    uint8_t gateway = msr->gateway;
    uint8_t devices[TEKON_PROTO_PLIST_SIZE];
    uint16_t addresses[TEKON_PROTO_PLIST_SIZE];
    uint16_t indexes[TEKON_PROTO_PLIST_SIZE];

    struct message msg_in;
    struct message msg_out;

    size_t i;
    int64_t tstamp = TIME_INVALID;

    for(i = 0; i < size; i++) {
        devices[i] = msr[i].device;
        addresses[i] = msr[i].address;
        indexes[i] = msr[i].index;
    }

    int result = tekon_req_1c(&msg_out, gateway, devices, addresses, indexes, size);

    if(!result) {
        log_print(APP_NAME " : can't create request\n");
        return 0;
    }

    result = process_request(&msg_out, &msg_in, link);

    const struct tekon_parameter * param = msg_in.payload.parameters;
    tstamp = time_now_utc();

    for(i = 0; i < size; i++, param++) {
        const uint32_t * value = NULL;
        enum quality qual = Q_NOCONN;
        size_t len = 0;
        if(result > 0 ) {
            value = &param->value;
            qual = param->qual == 0 ? Q_OK : Q_INVALID;
            len = 4;
        }
        msr_update(msr + i, qual, tstamp,
                   value, len);
    }
    return result;
}

/* Прочитать данные из устройства
 * 0 - в случае ошибки */
static int read(struct app * app)
{
    assert(app);
    struct msr_table * table = &app->table;
    struct link * link = &app->link;
    const struct netaddr * addr = &app->netcfg;
    const size_t lim = msr_table_size(table);
    size_t pos = 0;

    if(addr->type == LINK_TCP)
        link_init_tcp(link, addr->ip, addr->port, app->timeout);
    else if(addr->type == LINK_UDP)
        link_init_udp(link, addr->ip, addr->port, app->timeout);

    int result = link_up(link);

    int64_t now = time_now_utc();
    msr_table_foreach(table, apply_noconn, &now);

    if(result != 0) {
        log_print(APP_ERR " : connecting error %d\n", result);
        return 0;
    }

    while(pos < lim) {
        const size_t diff = lim - pos;
        const size_t chunk = diff == 1 ? 1 :
                             diff > TEKON_PROTO_PLIST_SIZE ? TEKON_PROTO_PLIST_SIZE :
                             diff;


        // Если порция данных была прочитана с ошибкой, то нет смысла читать
        // остальные. Просто стивим всем оставшимся ошибку связи. Чтобы
        // сделать это с минимальным трудом, кладем коннект и пытаемся
        // вычитать данные на отключенном линке.
        if(!read_multiple(msr_table_get(table, pos), chunk, link)) {
            link_down(link);
            return 0;
        }
        pos += chunk;
    }
    link_down(link);
    return 1;
}

/* Прочитать аргусенты командной строки
 * 0 - в случае ошибки */
static int read_args(struct app * app, int argc, char * const argv[])
{
    assert(app);

    if(argc < 4)
        return 0;

    int opt;
    uint8_t gateway = 0;
    struct paraddr param;

    while ((opt = getopt(argc, argv, "t:a:p:v:")) != -1) {
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
        case 'p': {
            if(gateway == 0) {
                printf("enter address before parameters list\n\n");
                return 0;
            }

            char * str = optarg;
            char * token = NULL;

            while((token = strtok_r(str, " ", &str))) {
                if(!paraddr_from_string(&param, token)) {
                    printf("invalid parameter address %s\n\n", optarg);
                    return 0;
                } else {
                    struct msr msr;
                    msr_init(&msr, gateway, param.device, param.address, param.index, param.type, param.hex);
                    if(!msr_table_add(&app->table, &msr)) {
                        printf("measurments overflow. Limit is %d\n\n", MEASURMENT_MAX_TABLE_SIZE);
                        return 0;
                    }
                }
            }
        }
        break;
        case 'a':
            if(!netaddr_from_string(&app->netcfg, optarg)) {
                printf("invalid network address %s\n\n", optarg);
                return 0;
            }
            gateway = app->netcfg.gateway;
            break;
        case 'v':
            log_setlevel(atoi(optarg));
            break;
        default: /* '?' */
            printf("invalid argument %c\n\n", opt);
            return 0;
        }
    }

    // Адрес не задан
    if(app->netcfg.port == 0) {
        printf("please enter gateway's address\n\n");
        return 0;
    }

    // Параметры не заданы
    if(msr_table_size(&app->table) == 0) {
        printf("please enter parameters to read\n\n");
        return 0;
    }

    return 1;
}

void print(struct msr * self, void * data)
{
    assert(self);
    assert(data);

    const struct app * app = data;
    char buffer[512] = {0};
    size_t remain = sizeof(buffer);
    char * ptr = buffer;

    // Добавить адрес шлюза
    int result = snprintf(ptr, remain, "%"PRIu8":",self->gateway);
    assert(result > 0);
    ptr += result;
    remain -= result;

    // Добавить адрес устройства
    result = snprintf(ptr, remain, "%"PRIu8":",self->device);
    assert(result > 0);
    ptr += result;
    remain -= result;

    // Добавить адрес параметра
    result = self->hex ?
             snprintf(ptr, remain, "0x%x ", self->address) :
             snprintf(ptr, remain, "%"PRIu16" ", self->address);
    assert(result > 0);
    ptr += result;
    remain -= result;

    // Добавить значение
    switch(self->type) {
    case TEKON_PARAM_RAW:
        result = snprintf(ptr, remain, "R 0x%02x%02x%02x%02x ",self->value.byte[0], self->value.byte[1], self->value.byte[2], self->value.byte[3]);
        break;
    case TEKON_PARAM_HEX:
        result = snprintf(ptr, remain, "H 0x%x ",self->value.u32);
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
    case TEKON_PARAM_TIME: {
        struct tekon_time tt;
        tekon_time_unpack(&tt, &self->value.u32, sizeof(self->value.u32));
        result = snprintf(ptr, remain, "T %02d:%02d:%02d ", tt.hour, tt.minute, tt.second);
    }
    break;
    case TEKON_PARAM_DATE: {
        struct tekon_date td;
        tekon_date_unpack(&td, &self->value.u32, sizeof(self->value.u32));
        result = snprintf(ptr, remain, "D %d-%02d-%02d ", td.year + 2000, td.month, td.day);
    }
    break;
    }
    assert(result > 0);
    ptr += result;
    remain -= result;

    // Добавить качество
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

    // Добавить метку времени
    result = snprintf(ptr, remain, "%"PRIi64" ", self->timestamp);
    assert(result > 0);
    ptr += result;
    remain -= result;

    // Добавить информацию о сдвиге часового пояса
    result = snprintf(ptr, remain, "%"PRIi32, app->tzoffset);
    assert(result > 0);

    // Вывести
    printf("%s\n", buffer);
}

int main(int argc, char * argv[])
{

    struct app app;
    init(&app);

    if(!read_args(&app, argc, argv)) {
        usage();
        return 1;
    }

    int result = read(&app);
    msr_table_foreach(&app.table, print, &app);
    return result == 0;

}

#ifdef __cplusplus
}
#endif
