/* Copyright (c) 2019
 * Alexander Shirokov
 * Schneider Electric
 * See LICENSE for details. */

#ifndef UTILS_SYNC_CHECK_H
#define UTILS_SYNC_CHECK_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <time.h>

/* Варианты проверки корректности нового времени. */
enum time_check {
    /* Время будет записано в устройство без проверки */
    TIME_CHECK_NONE = 0,

    /* Время будет записано, если новое время отличается от времени устройства не
     * более, чем на N мс*/
    TIME_CHECK_DIFF = 1,

    /* Время будет записано, если новое время не приведет к изменению индексов в
     * месячном, дневном, суточном и интервальном архивах. */
    TIME_CHECK_INDEX = 2,

    /* Время будет записано, если новое время не приводит к изменения минут,
     * часов, дней, месяцев, годов */
    TIME_CHECK_MINUTES = 4
};

#define CHECK_MSG_BUFFER 256
struct checks {
    enum time_check avail;
    enum time_check fail;

    int diff;
    int minutes;

    char message[CHECK_MSG_BUFFER];

};

void checks_init(struct checks * self);
int checks_from_string(struct checks * self, const char * str);
int checks_run(struct checks * self, const struct tm * newtime, const struct tm * devtime);


#ifdef __cplusplus
}
#endif

#endif
