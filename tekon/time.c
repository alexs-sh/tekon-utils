/* Copyright (c) 2019
 * Alexander Shirokov
 * Schneider Electric
 * See LICENSE for details. */

#ifdef __cplusplus
extern "C" {
#endif

#include "tekon/time.h"
#include <assert.h>
#include <string.h>

static const uint16_t DAYS_IN_YEAR[2][12] = {
    {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334},
    {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335},
};

static const uint16_t DAYS_IN_MONTH[2][12] = {
    {31,28,31,30,31,30,31,31,30,31,30,31},
    {31,29,31,30,31,30,31,31,30,31,30,31},
};

static int is_leap_year(uint8_t year)
{
    /* стандартное определение високосных годов:
     *   год, номер которого кратен 400, — високосный;
     *   остальные годы, номер которых кратен 100, — невисокосные;
     *   остальные годы, номер которых кратен 4, — високосные.
     *В Тэконе год может быть записан в формате 0..99 ->
     *первые 2 условия можно проигнорировать. */
    return year % 4 == 0;
}

static int year_is_valid(uint8_t year)
{
    return year < 100;
}

static int month_is_valid(uint8_t month)
{
    return month >= 1 && month <= 12;
}

static int day_is_valid(uint8_t year, uint8_t month, uint8_t day)
{
    const int is_leap = is_leap_year(year);
    return day >= 1 && day <= DAYS_IN_MONTH[is_leap][month-1];
}

static int hour_is_valid(uint8_t hour)
{
    return hour < 24;
}

static int minute_is_valid(uint8_t minute)
{
    return minute < 60;
}

static int second_is_valid(uint8_t second)
{
    return second < 60;
}

int tekon_month_index(uint8_t year, uint8_t month,uint8_t depth)
{
    if(!(year_is_valid(year) &&
            month_is_valid(month)))
        return TEKON_INVALID_ARCH_INDEX;

    if(depth == 12)
        return month - 1;
    else if (depth == 48)
        return (year % 4) * 12 + month - 1;

    return TEKON_INVALID_ARCH_INDEX;
}

int tekon_day_index(uint8_t year, uint8_t month, uint8_t day)
{
    if(!(year_is_valid(year) &&
            month_is_valid(month) &&
            day_is_valid(year, month, day)))
        return TEKON_INVALID_ARCH_INDEX;

    const int is_leap = is_leap_year(year);
    return DAYS_IN_YEAR[is_leap][month - 1] + day - 1;
}

int tekon_hour_index(uint8_t year, uint8_t month, uint8_t day, uint8_t hour, uint16_t depth)
{
    if(!(year_is_valid(year) &&
            month_is_valid(month) &&
            day_is_valid(year, month, day) &&
            hour_is_valid(hour)))
        return TEKON_INVALID_ARCH_INDEX;

    depth /= 24;
    if(!(depth == 16 || depth == 32 || depth == 64))
        return TEKON_INVALID_ARCH_INDEX;

    const int nday = 365*year +
                     year / 4 +
                     tekon_day_index(year, month, day) +
                     !is_leap_year(year);
    return (nday % depth) * 24 + hour;
}

int tekon_interval_index(uint8_t year, uint8_t month, uint8_t day,
                         uint8_t hour, uint8_t minute, uint16_t limit, uint16_t interval)
{
    if(!(year_is_valid(year) &&
            month_is_valid(month) &&
            day_is_valid(year, month, day) &&
            hour_is_valid(hour) &&
            minute_is_valid(minute) ))
        return TEKON_INVALID_ARCH_INDEX;

    const int depth = interval * limit / 1440;

    const int nday =
        365 * year +
        year / 4 +
        tekon_day_index(year, month, day) +
        !is_leap_year(year);

    return (60 / interval) * ((nday % depth) * 24 + hour) +
           (minute / interval);

}

int tekon_time_unpack(struct tekon_time * time, const void * buffer,size_t size)
{
    assert(time);
    assert(buffer);

    if(size < 4)
        return 0;

    const uint8_t * ptr = buffer;

    ptr++;
    time->second = (*ptr >> 4) * 10 + (*ptr & 0x0F);

    ptr++;
    time->minute = (*ptr >> 4) * 10 + (*ptr & 0x0F);

    ptr++;
    time->hour = (*ptr >> 4) * 10 + (*ptr & 0x0F);

    return 1;
}

int tekon_date_unpack(struct tekon_date * date, const void * buffer,size_t size)
{

    assert(date);
    assert(buffer);

    if(size < 4)
        return 0;

    const uint8_t * ptr = buffer;
    date->dow = (*ptr >> 4) * 10 + (*ptr & 0x0F);

    ptr++;
    date->day = (*ptr >> 4) * 10 + (*ptr & 0x0F);

    ptr++;
    date->month = (*ptr >> 4) * 10 + (*ptr & 0x0F);

    ptr++;
    date->year = (*ptr >> 4) * 10 + (*ptr & 0x0F);

    return 1;
}

int tekon_time_pack(const struct tekon_time * time, void * buffer, size_t size)
{
    assert(time);
    assert(buffer);
    if(size < 4)
        return 0;
    uint8_t * ptr = buffer;
    *ptr++ = 0;

    memset(buffer, 0, 4);
    *ptr = (time->second / 10 << 4) | time->second % 10;
    ptr++;

    *ptr |= (time->minute / 10 << 4) | time->minute % 10;
    ptr++;

    *ptr |= (time->hour / 10 << 4) | time->hour % 10;

    return 1;

}

int tekon_date_pack(const struct tekon_date * date, void * buffer, size_t size)
{
    assert(date);
    assert(buffer);

    uint8_t * ptr = buffer;

    memset(buffer, 0, 4);

    *ptr |= date->dow / 10 << 4 | date->dow % 10;
    ptr++;

    *ptr |= date->day / 10 << 4 | date->day % 10;
    ptr++;

    *ptr |= date->month / 10 << 4 | date->month % 10;
    ptr++;

    *ptr |= date->year / 10 << 4 | date->year % 10;
    return 1;

}

int tekon_date_is_valid(const struct tekon_date * date)
{
    return year_is_valid(date->year)   && month_is_valid(date->month) && day_is_valid(date->year, date->month, date->day);
}

int tekon_time_is_valid(const struct tekon_time * time)
{
    return hour_is_valid(time->hour) && minute_is_valid(time->minute) && second_is_valid(time->second);
}


#ifdef __cplusplus
}
#endif
