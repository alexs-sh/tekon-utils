/* Copyright (c) 2019
 * Alexander Shirokov
 * Schneider Electric
 * See LICENSE for details. */

#ifndef TEKON_TIME_H
#define TEKON_TIME_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

#define TEKON_INVALID_ARCH_INDEX 0xFFFF

struct tekon_time {
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
};

struct tekon_date {
    uint8_t year;
    uint8_t month;
    uint8_t day;
    uint8_t dow;
};

/* Функции для преобразования времени в индексы Тэкона.
 * Для Текона используются сл. форматы времени:
 * года [0,99]
 * месяцы [1,12]
 * дни [1,31]
 * часы [0,23]
 * минуты [0,59]
 * */

/* Т10.06.59РД-Д1 стр. 31 Б1,Б2,Б3
 * depth 12 или 48 */
int tekon_month_index(uint8_t year, uint8_t month,uint8_t depth);


/* Т10.06.59РД-Д1 стр. 31 Б4 */
int tekon_day_index(uint8_t year, uint8_t month, uint8_t day);

/* Т10.06.59РД-Д1 стр. 31 Б4 */
int tekon_day_index(uint8_t year, uint8_t month, uint8_t day);

/* Т10.06.59РД-Д1 стр. 32 Б5
 * depth 384 (16 дней), 764 (32 дня), 1536 (64 дня) */
int tekon_hour_index(uint8_t year, uint8_t month, uint8_t day, uint8_t hour, uint16_t depth);

/* Т10.06.59РД-Д1 стр. 32 Б6
 * interval - минуты */
int tekon_interval_index(uint8_t year, uint8_t month, uint8_t day,
                         uint8_t hour, uint8_t minute, uint16_t limit, uint16_t interval);



int tekon_time_unpack(struct tekon_time * time, const void * buffer, size_t size);
int tekon_date_unpack(struct tekon_date * date, const void * buffer, size_t size);
int tekon_time_pack(const struct tekon_time * time, void * buffer, size_t size);
int tekon_date_pack(const struct tekon_date * date, void * buffer, size_t size);

int tekon_date_is_valid(const struct tekon_date * date);
int tekon_time_is_valid(const struct tekon_time * time);

#ifdef __cplusplus
}
#endif

#endif
