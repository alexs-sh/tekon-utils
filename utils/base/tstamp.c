/* Copyright (c) 2019
 * Alexander Shirokov
 * Schneider Electric
 * See LICENSE for details. */

#ifdef __cplusplus
extern "C" {
#endif
#include "utils/base/tstamp.h"
#include <assert.h>
#include <string.h>
#include "tekon/time.h"
#include "utils/base/time.h"



static int timestamp_seq_add(struct timestamp_seq * self, size_t index, int64_t tstamp)
{
    assert(self);

    if(index >= TIMESTAMP_MAX_SEQ_SIZE)
        return 0;

    if(self->count >= TIMESTAMP_MAX_SEQ_SIZE)
        return 0;

    int64_t * pts = self->time + index;
    if(*pts == TIME_INVALID) {
        *pts = tstamp;
        self->count++;
        return 1;
    }

    return 0;
}

int tekon_time_to_local(const struct tekon_time * tekon, struct tm * local)
{
    assert(local);
    assert(tekon);

    if(!tekon_time_is_valid(tekon))
        return 0;

    local->tm_hour = tekon->hour;
    local->tm_min = tekon->minute;
    local->tm_sec = tekon->second;

    return 1;
}

int tekon_time_from_local(struct tekon_time * tekon, const struct tm * local)
{
    assert(local);
    assert(tekon);

    tekon->hour = local->tm_hour ;
    tekon->minute = local->tm_min;
    tekon->second = local->tm_sec;

    return tekon_time_is_valid(tekon);
}

int tekon_date_to_local(const struct tekon_date * tekon, struct tm * local)
{
    assert(local);
    assert(tekon);

    if(!tekon_date_is_valid(tekon))
        return 0;

    local->tm_year = tekon->year + 100;
    local->tm_mon = tekon->month - 1;
    local->tm_mday = tekon->day;

    return 1;
}

int tekon_date_from_local(struct tekon_date * tekon, const struct tm * local)
{
    assert(local);
    assert(tekon);

    tekon->year = local->tm_year % 100;
    tekon->month = local->tm_mon + 1;
    tekon->day = local->tm_mday;
    tekon->dow = local->tm_wday == 0 ? 6 : local->tm_wday - 1;

    return 1;
}

void timestamp_seq_init(struct timestamp_seq * self)
{
    assert(self);
    size_t i;
    int64_t * ts = self->time;
    for(i = 0; i < TIMESTAMP_MAX_SEQ_SIZE; i++, ts++) {
        *ts = TIME_INVALID;
    }
    self->count = 0;
}

int64_t timestamp_seq_get(const struct timestamp_seq * self, size_t index)
{
    assert(self);

    if(index >= TIMESTAMP_MAX_SEQ_SIZE)
        return TIME_INVALID;

    return self->time[index];

}

size_t timestamp_seq_size(const struct timestamp_seq * self)
{
    return self->count;
}

int timestamp_seq_month(struct timestamp_seq * self, const struct tekon_date * date, size_t depth)
{
    assert(self);

    timestamp_seq_init(self);

    if(!(depth == 12 || depth == 48))
        return 0;

    const int step = 3600*24*8; /* шаг 8 дней в сек. */
    const int64_t limit = depth * 31 * 24 * 3600; /* мес * дни * часы * сек */
    int64_t elapsed = 0;
    int64_t utc = 0;

    struct tm dt;
    memset(&dt, 0, sizeof(dt));

    /* 1. получить стартовую дату/время.
     * Начальная на 1 месяц меньше date */
    if(!tekon_date_to_local(date, &dt))
        return 0;

    const int month = dt.tm_mon;
    do {
        utc = time_utc_from_local(&dt);
        if(utc == TIME_INVALID)
            return 0;
        utc -= step;
        if(time_local_from_utc(utc, &dt) < 0)
            return 0;
    } while(month == dt.tm_mon);

    /*2. Последовательно создаем метки времени. Для каждой из них вычисляем
     * локальное время, затем индекс тэкона. Если это новый индекс, то сохраянем
     * индекс и метку времени c округлением по границам архива.
     * Выполняем пока не наберем все индексы или пока не выйдем за допустимый
     * дипазон дат */
    do {

        struct tekon_date tekon;
        tekon_date_from_local(&tekon, &dt);

        int probe = tekon_month_index(tekon.year, tekon.month, depth);
        if(timestamp_seq_get(self, probe) == TIME_INVALID) {
            dt.tm_mday = 1;
            dt.tm_hour = 0;
            dt.tm_min = 0;
            dt.tm_sec = 0;
            tekon_date_from_local(&tekon, &dt);
            int idx = tekon_month_index(tekon.year, tekon.month, depth);
            assert(idx == probe);
            utc = time_utc_from_local(&dt);
            timestamp_seq_add(self, idx, utc);
        }

        utc -= step;
        elapsed += step;

        if(time_local_from_utc(utc, &dt) < 0)
            return 0;

    } while(timestamp_seq_size(self) < depth && elapsed < limit);

    return timestamp_seq_size(self) == depth;
}


int timestamp_seq_day(struct timestamp_seq * self, const struct tekon_date * date)
{
    assert(self);

    timestamp_seq_init(self);

    const size_t size = 366;
    const int step = 8*3600;
    const int64_t limit = size * 24 * 3600;
    int64_t elapsed = 0;
    int64_t utc = 0;

    struct tm dt;
    memset(&dt, 0, sizeof(dt));

    /* 1. получить стартовую дату/время.
     * Начальная на 1 день меньше date */
    if(!tekon_date_to_local(date, &dt))
        return 0;

    const int day = dt.tm_mday;
    do {
        utc = time_utc_from_local(&dt);
        if(utc == TIME_INVALID)
            return 0;
        utc -= step;
        if(time_local_from_utc(utc, &dt) < 0)
            return 0;
    } while(day == dt.tm_mday);

    /*2. Последовательно создаем метки времени. Для каждой из них вычисляем
     * локальное время, затем индекс тэкона. Если это новый индекс, то сохраянем
     * индекс и метку времени c округлением по границам архива.
     * Выполняем пока не наберем все индексы или пока не выйдем за допустимый
     * дипазон дат */
    do {
        struct tekon_date tekon;
        tekon_date_from_local(&tekon, &dt);

        int probe = tekon_day_index(tekon.year, tekon.month, tekon.day);
        if(timestamp_seq_get(self, probe) == TIME_INVALID) {
            dt.tm_hour = 0;
            dt.tm_min = 0;
            dt.tm_sec = 0;

            tekon_date_from_local(&tekon, &dt);
            int idx = tekon_day_index(tekon.year, tekon.month, tekon.day);
            assert(idx == probe);
            utc = time_utc_from_local(&dt);
            timestamp_seq_add(self, idx, utc);
        }

        utc -= step;
        elapsed += step;

        if(time_local_from_utc(utc, &dt) < 0)
            return 0;

    } while(timestamp_seq_size(self) < size && elapsed < limit);

    return timestamp_seq_size(self) >= 365;
}

int timestamp_seq_hour(struct timestamp_seq * self, const struct tekon_date * date, const struct tekon_time * time, size_t depth)
{
    assert(self);

    timestamp_seq_init(self);
    if (!(depth == 384 || depth == 768 || depth == 1536))
        return 0;

    const int step = 1200;
    const int64_t limit = depth * 3600;
    int64_t elapsed = 0;
    int64_t utc = 0;

    struct tm dt;
    memset(&dt, 0, sizeof(dt));

    /* 1. получить стартовую дату/время.
     * Начальная на 1 час меньше date */
    if(!tekon_date_to_local(date, &dt))
        return 0;

    if(!tekon_time_to_local(time, &dt))
        return 0;

    const int hour = dt.tm_hour;
    do {
        utc = time_utc_from_local(&dt);
        if(utc == TIME_INVALID)
            return 0;
        utc -= step;
        if(time_local_from_utc(utc, &dt) < 0)
            return 0;
    } while(hour == dt.tm_hour);

    /*2. Последовательно создаем метки времени. Для каждой из них вычисляем
     * локальное время, затем индекс тэкона. Если это новый индекс, то сохраянем
     * индекс и метку времени c округлением по границам архива.
     * Выполняем пока не наберем все индексы или пока не выйдем за допустимый
     * дипазон дат */
    do {

        struct tekon_date tekon_dt;
        struct tekon_time tekon_tm;

        tekon_time_from_local(&tekon_tm, &dt);
        tekon_date_from_local(&tekon_dt, &dt);

        /* Проверяем метку времени. Если она еще не добавлена, то выполняем
           округление и добавляем */
        int probe = tekon_hour_index(tekon_dt.year, tekon_dt.month, tekon_dt.day, tekon_tm.hour, depth);
        if(timestamp_seq_get(self, probe) == TIME_INVALID) {
            dt.tm_min = 0;
            dt.tm_sec = 0;

            tekon_time_from_local(&tekon_tm, &dt);
            tekon_date_from_local(&tekon_dt, &dt);

            int idx = tekon_hour_index(tekon_dt.year, tekon_dt.month, tekon_dt.day, tekon_tm.hour, depth);
            assert(idx == probe);
            utc = time_utc_from_local(&dt);
            timestamp_seq_add(self, idx, utc);
        }

        utc -= step;
        elapsed += step;

        if(time_local_from_utc(utc, &dt) < 0)
            return 0;

    } while(timestamp_seq_size(self) < depth && elapsed < limit);

    return timestamp_seq_size(self) == depth;

}

int timestamp_seq_interval(struct timestamp_seq * self, const struct tekon_date * date, const struct tekon_time * time, size_t depth, size_t interval)
{
    assert(self);

    if(depth > TIMESTAMP_MAX_SEQ_SIZE)
        return 0;

    timestamp_seq_init(self);

    const size_t limit = depth * interval * 60; /* лимит в секундах */
    const int step = interval * 60;

    size_t elapsed = 0;
    struct tm dt;

    memset(&dt, 0, sizeof(dt));

    /* Заполнить дату/время. */
    if(!tekon_date_to_local(date, &dt))
        return 0;

    if(!tekon_time_to_local(time, &dt))
        return 0;

    int64_t utc = time_utc_from_local(&dt);
    if(utc == TIME_INVALID)
        return 0;

    /* Оркгулить по границе интервала в большую сторону */
    utc = (utc / step + 1) * step;
    do {
        struct tm ldt;
        if(time_local_from_utc(utc, &ldt) < 0)
            return 0;

        struct tekon_date tekon_dt;
        struct tekon_time tekon_tm;

        tekon_time_from_local(&tekon_tm, &ldt);
        tekon_date_from_local(&tekon_dt, &ldt);

        int idx = tekon_interval_index(tekon_dt.year, tekon_dt.month, tekon_dt.day,
                                       tekon_tm.hour, tekon_tm.minute,
                                       depth, interval);

        timestamp_seq_add(self, idx, utc);

        utc -= step;
        elapsed += step;

    } while(timestamp_seq_size(self) < depth && elapsed < limit);

    return timestamp_seq_size(self) == depth;

}

#ifdef __cplusplus
}
#endif

