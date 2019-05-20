/* Copyright (c) 2019
 * Alexander Shirokov
 * Schneider Electric
 * See LICENSE for details. */

#ifndef UTILS_BASE_TIME_H
#define UTILS_BASE_TIME_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <time.h>

#define TIME_INVALID (-1)

/*Вернуть UTC время в сек.*/
int64_t time_now_utc();

/*Вернуть локальное время в сек.*/
int64_t time_now_local();

/* получить сдвиг часового пояса от UTC (сек) */
int32_t time_tzoffset();

/*Сгенерировать локальную дату/время из UTC */
int time_local_from_utc(int64_t utc, struct tm * local);

/*Сгенерировать UTC время из локальной даты/времени*/
int64_t time_utc_from_local(const struct tm * local);


#ifdef __cplusplus
}
#endif

#endif
