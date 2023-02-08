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
#include <signal.h>
#include <getopt.h>
#include <inttypes.h>

#include "utils/base/base.h"
#include "utils/sync/check.h"
#include "tekon/tekon.h"

#define APP_NAME "tekon_sync"
#define APP_ERR  LOG_ERR  APP_NAME " : ERR"
#define APP_WARN LOG_WARN APP_NAME " : WARN"
#define APP_INFO LOG_WARN APP_NAME " : INFO"

struct app {
    struct netaddr netcfg;
    struct link link;
    struct dtaddr dtaddr;
    struct checks checks;
    uint8_t password[TEKON_PASSWD_LEN];
    int64_t newtime; /* Устанавливаемое время (UTC) */
    int allow_diff;
    int timeout;
};


static void init(struct app * self)
{
    assert(self);
    memset(self, 0, sizeof(*self));
    checks_init(&self->checks);
    self->timeout = 1000;
    self->newtime = time_now_utc();
}

static void usage()
{
    printf("Usage: %s -a address -d date/time -p password\n", APP_NAME);
    printf("                  [-t timeout] [-u time] [-c checks] [-v verbosity]\n\n");
    printf("  -a    gateway's address in [type:ip:port@gateway] format.\n\n");
    printf("  -d    date/time addresses in [device:dateaddr:timeaddr] format.\n\n");
    printf("  -t    response timeout in milliseconds.\n\n");
    printf("  -u    new time in UTC. Use host time by default.\n\n");
    printf("  -p    password [8 digits].\n\n");
    printf("  -c    time's checks parameters:\n\n");
    printf("        none - disable checks.\n\n");
    printf("        indexes - ensures that indexes won't change during time\n");
    printf("        synchronization.\n\n");
    printf("        difference:N - ensures that new time differs from device\n");
    printf("        time not more than N seconds.\n\n");
    printf("        minutes:N - ensures that new time doesn't break interval\n");
    printf("        of N-minutes.\n\n");
    printf("  -v    set verbose:\n");
    printf("        0 - silent\n");
    printf("        1 - error [default]\n");
    printf("        2 - warning\n");
    printf("        3 - info\n\n");
    printf("Example:\n");

    printf("  %s -a udp:10.0.0.3:51960@9 -d 3:0xF017:0xF018 -p00000001 -u 1557833011\n", APP_NAME);
    printf("  %s -a udp:10.0.0.3:51960@9 -d 3:0xF017:0xF018 -p00000001 -u 1557833011 -c 'difference:100 minutes:1 indexes'\n", APP_NAME);
    printf("  DT=$(date -u +%%s) &&  %s -a udp:10.0.0.3:51960@9 -d 3:0xF017:0xF018  -v3  -p00000001 -u ${DT} -c none\n", APP_NAME);
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

    if(response->type == TEKON_MSG_POS_ACK)
        return 1;

    if(response->type == TEKON_MSG_WRITEM_PAR_14)
        return nin == nout;

    return  nin == nout && request->nelements == response->nelements;
}

/* Чтение времени из устройства
 * 0 - в случае ошибки */
static int read_time(struct tm * devtime, const struct dtaddr * dtaddr, struct link * link)
{

    uint8_t size = 2;
    uint8_t devices[2] = {dtaddr->device, dtaddr->device};
    uint16_t addresses[2] = {dtaddr->date,dtaddr->time};
    uint16_t indexes[2] = {0};

    struct tekon_date date;
    struct tekon_time time;

    struct message request;
    struct message response;

    int result = tekon_req_1c(&request, dtaddr->gateway, devices, addresses, indexes, size);
    if(!result) {
        log_print(APP_ERR " : can't create request\n");
        return 0;
    }

    result = process_request(&request, &response, link) > 0 &&
             tekon_date_unpack(&date, &response.payload.parameters[0].value, 4) &&
             tekon_time_unpack(&time, &response.payload.parameters[1].value, 4);

    if(!result) {
        log_print(APP_ERR " : date/time reading failed\n");
        return 0;
    }

    if(!tekon_date_to_local(&date, devtime)) {
        log_print(APP_ERR " : device date in invalid\n");
        return 0;
    }

    if(!tekon_time_to_local(&time, devtime)) {
        log_print(APP_ERR " : device time in invalid\n");
        return 0;
    }

    return 1;
}

/* Установка прав доступа к устройству
 * 0 - в случае ошибки */
static int login(struct link * link, uint8_t gateway, uint8_t device, uint8_t level, const uint8_t passwd[TEKON_PASSWD_LEN])
{
    const uint8_t cmd[8] = {0x07, device, 0x05, level,
                            passwd[0] * 10 + passwd[1],
                            passwd[2] * 10 + passwd[3],
                            passwd[4] * 10 + passwd[5],
                            passwd[6] * 10 + passwd[7]
                           };

    struct message request;
    struct message response;

    int result = tekon_req_14(&request, gateway, cmd, sizeof(cmd));
    if(!result) {
        log_print(APP_ERR " : can't create request\n");
        return 0;
    }

    result = process_request(&request, &response, link) > 0;

    if(result)
        return response.type == TEKON_MSG_POS_ACK || response.type == TEKON_MSG_WRITEM_PAR_14;

    log_print(APP_ERR " : login failed\n");
    return 0;
}

/* Запись времени в устройство
 * 0 - в случае ошибки */
static int write_time(struct link * link, const struct dtaddr * addr, const struct tm * newtime)
{
    struct tekon_date date;
    struct tekon_time time;

    uint32_t packed_date;
    uint32_t packed_time;

    uint8_t cmd_date[9] = {0x08, addr->device, 0x03, 0x00, 0x00, 0x00,0x00,0x00,0x00};
    uint8_t cmd_time[9] = {0x08, addr->device, 0x03, 0x00, 0x00, 0x00,0x00,0x00,0x00};

    tekon_date_from_local(&date, newtime);
    tekon_time_from_local(&time, newtime);

    tekon_date_pack(&date, &packed_date, sizeof(packed_date));
    tekon_time_pack(&time, &packed_time, sizeof(packed_time));

    memcpy(cmd_date + 3, &addr->date, sizeof(addr->date));
    memcpy(cmd_date + 5, &packed_date, sizeof(packed_date));

    memcpy(cmd_time + 3, &addr->time, sizeof(addr->time));
    memcpy(cmd_time + 5, &packed_time, sizeof(packed_time));

    struct message request;
    struct message response;

    int result = tekon_req_14(&request, addr->gateway, cmd_date, sizeof(cmd_date));

    if(!result) {
        log_print(APP_ERR " : can't create date writing request\n");
        return 0;
    }

    result = process_request(&request, &response, link) > 0;
    if(!result) {
        log_print(APP_ERR " : date writing failed\n");
        return 0;
    }

    result = tekon_req_14(&request, addr->gateway, cmd_time, sizeof(cmd_time));
    if(!result) {
        log_print(APP_ERR " : can't create time writing request\n");
        return 0;
    }

    result = process_request(&request, &response, link) > 0;
    if(!result) {
        log_print(APP_ERR " : time writing failed\n");
        return 0;
    }

    return 1;
}

/* Функция, управляющая записью времени в устройство.
 * Выполняет запись + подготовительные/завершающие шаги
 * 0 - в случае ошибки */
static int sync_time(struct app * app)
{
    assert(app);
    struct link * link = &app->link;
    const struct netaddr * addr = &app->netcfg;

    if(addr->type == LINK_TCP)
        link_init_tcp(link, addr->ip, addr->port, app->timeout);
    else if(addr->type == LINK_UDP)
        link_init_udp(link, addr->ip, addr->port, app->timeout);

    int result = link_up(link);

    if(result != 0) {
        log_print(APP_ERR " : connecting error %d\n", result);
        return 0;
    }

    struct tm devtime;
    struct tm newtime;
    memset(&devtime, 0, sizeof(devtime));
    memset(&newtime, 0, sizeof(newtime));

    time_local_from_utc(app->newtime, &newtime);

    /* 1. Прочитать текущее время */
    result = read_time(&devtime, &app->dtaddr, &app->link);
    if(!result) {
        link_down(&app->link);
        return 0;
    }

    log_print(APP_INFO " : device UTC %"PRIi64"\n", time_utc_from_local(&devtime));
    log_print(APP_INFO " : device time %d-%02d-%02d %02d:%02d:%02d\n",
              devtime.tm_year + 1900,
              devtime.tm_mon + 1,
              devtime.tm_mday,
              devtime.tm_hour,
              devtime.tm_min,
              devtime.tm_sec);

    log_print(APP_INFO " : new UTC %"PRIi64"\n", app->newtime);
    log_print(APP_INFO " : new time %d-%02d-%02d %02d:%02d:%02d\n",
              newtime.tm_year + 1900,
              newtime.tm_mon + 1,
              newtime.tm_mday,
              newtime.tm_hour,
              newtime.tm_min,
              newtime.tm_sec);

    /* 2. Проверить время устройства и задаваемое время. Новое время должно
     * удовлетворять всем сконфигурированным условиям. Это позвоялет снизить
     * риск повреждения архивов */
    result = checks_run(&app->checks, &newtime, &devtime);

    if(!result) {
        log_print(APP_ERR " : %s\n", app->checks.message);
        link_down(&app->link);
        return 0;
    }

    /* 3. Для записи времени необходимо получить права наладчика (админа) */
    result = login(&app->link, app->netcfg.gateway, app->dtaddr.device, TEKON_PRIV_ADMIN, app->password);
    if(!result) {
        link_down(&app->link);
        return 0;
    }

    /* 4. Непосредственно запись времени */
    write_time(&app->link, &app->dtaddr, &newtime);

    /* 5. Сбросить админские права. Если мы не сделаем этого, Тэкон сам сбросит
     * права через какое-то время. Но нам не сложно... */
    result = login(&app->link, app->netcfg.gateway, app->dtaddr.device, TEKON_PRIV_USER, app->password);
    if(!result) {
        link_down(&app->link);
        return 0;
    }

    link_down(link);
    return 1;
}

/* Чтение аргументов комагдной строки
 * 0 - в случае ошибки */
static int read_args(struct app * app, int argc, char * const argv[])
{
    assert(app);

    if(argc < 2)
        return 0;

    int opt;
    int passread = 0;
    while ((opt = getopt(argc, argv, "t:a:d:p:v:u:c:")) != -1) {
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
        case 'u': {
            long input  = atol(optarg);
            if(input < 0) {
                printf("invalid UTC value %s\n\n", optarg);
                return 0;
            }
            app->newtime = input;
        }
        break;
        case 'c':
            if(!checks_from_string(&app->checks, optarg)) {
                printf("invalid checks %s\n\n", optarg);
                return 0;
            }
            break;
        case 'p':
            if(strnlen(optarg, TEKON_PASSWD_LEN) != TEKON_PASSWD_LEN) {
                printf("invalid password length \n\n");
                return 0;
            }
            for(int i = 0; i < TEKON_PASSWD_LEN; i++)
                app->password[i] = optarg[i] - '0';
            passread = 1;
            break;
        case 'a':
            if(!netaddr_from_string(&app->netcfg, optarg)) {
                printf("invalid network address %s\n\n", optarg);
                return 0;
            }
            break;
        case 'd':
            if(!dtaddr_from_string(&app->dtaddr, optarg)) {
                printf("invalid date/time address %s\n\n", optarg);
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

    /* Пароль не задан */
    if(!passread) {
        printf("please enter password\n\n");
        return 0;
    }


    /* Адрес не задан */
    if(app->netcfg.port == 0) {
        printf("please enter gateway's address\n\n");
        return 0;
    }

    app->dtaddr.gateway = app->netcfg.gateway;

    /* Адрес не задан */
    if(app->dtaddr.date == 0) {
        printf("please enter date's address\n\n");
        return 0;
    }

    /* Адрес не задан */
    if(app->dtaddr.time == 0) {
        printf("please enter time's address\n\n");
        return 0;
    }
    return 1;
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

    if(!read_args(&app, argc, argv)) {
        usage();
        return 1;
    }

    int result = sync_time(&app);
    return result == 0;
}

#ifdef __cplusplus
}
#endif
