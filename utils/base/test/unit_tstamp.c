/* Copyright (c) 2019
 * Alexander Shirokov
 * Schneider Electric
 * See LICENSE for details. */

#ifdef __cplusplus
extern "C" {
#endif

#include "test/minunit.h"
#include "utils/base/tstamp.h"
#include "utils/base/time.h"
#include "tekon/time.h"


MU_TEST(test_month_12)
{
    const int depth = 12;
    struct timestamp_seq seq;
    struct tekon_date date = {.year = 19, .month = 12, .day = 1};
    int result = timestamp_seq_month(&seq,&date, depth);
    mu_assert_int_eq(1, result);
    mu_assert_int_eq(depth, timestamp_seq_size(&seq));

    // Проверка для месячной последовательности может быть выполнена просто:
    // индекс текона = № месяца - 1
    // Соответсвенно, можно взять метку времени по индексу, преобразовать в
    // локальное время и убедится, что номер месяца совпадает

    // Кроме того, можно убедится, что даты, сгенерированные по индексу, дают нам тот же самый индекс
    size_t i;
    for(i = 0; i < timestamp_seq_size(&seq); i++) {
        struct tm ldt;
        time_local_from_utc(timestamp_seq_get(&seq, i), &ldt);
        mu_assert_int_eq(i, ldt.tm_mon);
        mu_assert_int_eq(i, tekon_month_index(ldt.tm_year % 100, ldt.tm_mon + 1, depth));
    }
}

MU_TEST(test_month_48)
{
    const int depth = 48;
    struct timestamp_seq seq;
    struct tekon_date date = {.year = 19, .month = 12, .day = 1};
    int result = timestamp_seq_month(&seq,&date, depth);
    mu_assert_int_eq(1, result);
    mu_assert_int_eq(depth, timestamp_seq_size(&seq));

    // убедится, что даты, сгенерированные по индексу, дают нам тот же самый индекс
    size_t i;
    for(i = 0; i < timestamp_seq_size(&seq); i++) {
        struct tm ldt;
        time_local_from_utc(timestamp_seq_get(&seq, i), &ldt);
        mu_assert_int_eq(i, tekon_month_index(ldt.tm_year % 100, ldt.tm_mon + 1, depth));
    }
}

MU_TEST(test_month_cv)
{
    // Проверка контроьного значения. Индекс взят и время взяты из реального
    // обмена.
    // время начала архива: 2018/09/01 00:00:00
    // время окончания архива: 2018/10/01 00:00:00
    const size_t index = 0x08;
    struct timestamp_seq seq;
    struct tekon_date date = {.year = 18, .month = 10, .day = 1};

    struct tm dt;
    memset(&dt, 0, sizeof(dt));

    dt.tm_year = date.year + 100; //tm_year: год - 1900
    dt.tm_mon = date.month - 2;   //tm_mon [0,11] + начало архива на 1 месяц раньше
    dt.tm_mday = date.day;
    dt.tm_hour = 0;
    dt.tm_min = 0;
    dt.tm_sec = 0;

    const int64_t tstamp = time_utc_from_local(&dt);
    timestamp_seq_month(&seq, &date, 12);
    mu_assert_int_eq(tstamp, timestamp_seq_get(&seq, index));
}


MU_TEST(test_day_365)
{
    struct timestamp_seq seq;
    struct tekon_date date = {.year = 19, .month = 12, .day = 31};
    int result = timestamp_seq_day(&seq,&date);
    mu_assert_int_eq(1, result);
    mu_assert_int_eq(365, timestamp_seq_size(&seq));

    // убедится, что даты, сгенерированные по индексу, дают нам тот же самый индекс
    size_t i;
    for(i = 0; i < timestamp_seq_size(&seq); i++) {
        struct tm ldt;
        time_local_from_utc(timestamp_seq_get(&seq, i), &ldt);
        mu_assert_int_eq(i, tekon_day_index(ldt.tm_year % 100, ldt.tm_mon + 1, ldt.tm_mday));
    }
}

MU_TEST(test_day_366)
{
    /* date - дата окончания архива. Для високосного 2016 года дата окончания
     * будет 2017.01.01 (см. Т10.06.59.РД-Д1)*/
    struct timestamp_seq seq;
    struct tekon_date date = {.year = 17, .month = 1, .day = 1};
    int result = timestamp_seq_day(&seq,&date);
    mu_assert_int_eq(1, result);
    mu_assert_int_eq(366, timestamp_seq_size(&seq));

    // убедится, что даты, сгенерированные по индексу, дают нам тот же самый индекс
    size_t i;
    for(i = 0; i < timestamp_seq_size(&seq); i++) {
        struct tm ldt;
        time_local_from_utc(timestamp_seq_get(&seq, i), &ldt);
        mu_assert_int_eq(i, tekon_day_index(ldt.tm_year % 100, ldt.tm_mon + 1, ldt.tm_mday));
    }
}

MU_TEST(test_day_trans)
{
    struct timestamp_seq seq;
    struct tekon_date date = {.year = 20, .month = 6, .day = 15};
    int result = timestamp_seq_day(&seq,&date);
    mu_assert_int_eq(1, result);
    mu_assert_int_eq(365, timestamp_seq_size(&seq));

    // убедится, что даты, сгенерированные по индексу, дают нам тот же самый индекс
    size_t i;
    for(i = 0; i < timestamp_seq_size(&seq); i++) {
        struct tm ldt;
        time_local_from_utc(timestamp_seq_get(&seq, i), &ldt);
        mu_assert_int_eq(i, tekon_day_index(ldt.tm_year % 100, ldt.tm_mon + 1, ldt.tm_mday));
    }
}

MU_TEST(test_day_cv)
{
    // Проверка контроьного значения. Индекс взят и время взяты из реального
    // обмена.
    // время начала архива: 2018/05/08 00:00:00
    // время окончания архива: 2018/05/09 00:00:00
    const size_t index = 0x7F;
    struct timestamp_seq seq;
    struct tekon_date date = {.year = 18, .month = 5, .day = 9};

    struct tm dt;
    memset(&dt, 0, sizeof(dt));

    dt.tm_year = date.year + 100; //tm_year: год - 1900
    dt.tm_mon = date.month - 1;   //tm_mon [0,11]
    dt.tm_mday = date.day - 1;    //начало архива на день раньше
    dt.tm_hour = 0;
    dt.tm_min = 0;
    dt.tm_sec = 0;

    const int64_t tstamp = time_utc_from_local(&dt);
    timestamp_seq_day(&seq, &date);
    mu_assert_int_eq(tstamp, timestamp_seq_get(&seq, index));
}


MU_TEST(test_hour_16)
{
    const uint16_t depth = 384;
    struct timestamp_seq seq;
    struct tekon_date date = {.year = 16, .month = 12, .day = 31};
    struct tekon_time time = {.hour = 16, .minute = 12, .second = 0};

    int result = timestamp_seq_hour(&seq, &date, &time, depth);
    mu_assert_int_eq(1, result);
    mu_assert_int_eq(depth, timestamp_seq_size(&seq));

    // убедится, что даты, сгенерированные по индексу, дают нам тот же самый индекс
    size_t i;
    for(i = 0; i < timestamp_seq_size(&seq); i++) {
        struct tm ldt;
        time_local_from_utc(timestamp_seq_get(&seq, i), &ldt);
        mu_assert_int_eq(i, tekon_hour_index(ldt.tm_year % 100, ldt.tm_mon + 1,
                                             ldt.tm_mday, ldt.tm_hour, depth));
    }
}

MU_TEST(test_hour_32)
{
    const uint16_t depth = 768;
    struct timestamp_seq seq;
    struct tekon_date date = {.year = 19, .month = 6, .day = 1};
    struct tekon_time time = {.hour = 0, .minute = 0, .second = 0};

    int result = timestamp_seq_hour(&seq, &date, &time, depth);
    mu_assert_int_eq(1, result);
    mu_assert_int_eq(depth, timestamp_seq_size(&seq));

    // убедится, что даты, сгенерированные по индексу, дают нам тот же самый индекс
    size_t i;
    for(i = 0; i < timestamp_seq_size(&seq); i++) {
        struct tm ldt;
        time_local_from_utc(timestamp_seq_get(&seq, i), &ldt);
        mu_assert_int_eq(i, tekon_hour_index(ldt.tm_year % 100, ldt.tm_mon + 1,
                                             ldt.tm_mday, ldt.tm_hour, depth));
    }
}

MU_TEST(test_hour_64)
{
    const uint16_t depth = 1536;
    struct timestamp_seq seq;
    struct tekon_date date = {.year = 19, .month = 1, .day = 1};
    struct tekon_time time = {.hour = 23, .minute = 59, .second = 59};

    int result = timestamp_seq_hour(&seq, &date, &time, depth);
    mu_assert_int_eq(1, result);
    mu_assert_int_eq(depth, timestamp_seq_size(&seq));

    // убедится, что даты, сгенерированные по индексу, дают нам тот же самый индекс
    size_t i;
    for(i = 0; i < timestamp_seq_size(&seq); i++) {
        struct tm ldt;
        time_local_from_utc(timestamp_seq_get(&seq, i), &ldt);
        mu_assert_int_eq(i, tekon_hour_index(ldt.tm_year % 100, ldt.tm_mon + 1,
                                             ldt.tm_mday, ldt.tm_hour, depth));
        //printf("%d %d\n", i, seq.time[i]);
    }
}

MU_TEST(test_hour_cv)
{
    // Проверка контроьного значения. Индекс взят и время взяты из реального
    // обмена.
    // время начала архива: 2019/03/05 14:00:00
    // время окончания архива: 2019/03/05 15:00:00
    const uint16_t depth = 1536;
    const size_t index = 0x296;

    struct timestamp_seq seq;
    struct tekon_date date = {.year = 19, .month = 3, .day = 5};
    struct tekon_time time = {.hour = 15, .minute = 0, .second = 0};

    struct tm dt;
    memset(&dt, 0, sizeof(dt));

    dt.tm_year = date.year + 100; //tm_year: год - 1900
    dt.tm_mon = date.month - 1;   //tm_mon [0,11]
    dt.tm_mday = date.day;
    dt.tm_hour = time.hour - 1;   //начало архива на час раньше
    dt.tm_min = time.minute;
    dt.tm_sec = time.second;

    const int64_t tstamp = time_utc_from_local(&dt);
    timestamp_seq_hour(&seq, &date, &time, depth);
    mu_assert_int_eq(tstamp, timestamp_seq_get(&seq, index));
}


MU_TEST(test_interval_5)
{
    const uint16_t depth = 1440;
    const uint16_t interval = 5;
    struct timestamp_seq seq;
    struct tekon_date date = {.year = 19, .month = 1, .day = 1};
    struct tekon_time time = {.hour = 23, .minute = 59, .second = 59};

    int result = timestamp_seq_interval(&seq, &date, &time, depth, interval);
    mu_assert_int_eq(1, result);
    mu_assert_int_eq(depth, timestamp_seq_size(&seq));

    // убедится, что даты, сгенерированные по индексу, дают нам тот же самый индекс
    size_t i;
    for(i = 0; i < timestamp_seq_size(&seq); i++) {
        struct tm ldt;
        time_local_from_utc(timestamp_seq_get(&seq, i), &ldt);
        mu_assert_int_eq(i, tekon_interval_index(ldt.tm_year % 100, ldt.tm_mon + 1,
                         ldt.tm_mday, ldt.tm_hour, ldt.tm_min, depth, interval));
        //printf("%d %d\n", i, seq.time[i]);
    }
}

MU_TEST(test_interval_cv)
{
    // Проверка контроьного значения. Индекс взят и время взяты из реального
    // обмена.
    // время начала архива: 2019/05/06 15:45:00
    // время окончания архива: 2019/05/06 15:50:00
    // В отличие от архивов остальных видов, считанные данные относятся к
    // интервалу, который закончился в ЧЧ / ММ  (Т10.06.59РД-Д1)
    // Т.е. контрольное время можно не сдвигать на начало архива
    const size_t index = 0xBD;
    const size_t depth = 1440;
    const size_t interval = 5;
    struct timestamp_seq seq;
    struct tekon_date date = {.year = 19, .month = 5, .day = 6};
    struct tekon_time time = {.hour = 15, .minute = 45, .second = 0};

    struct tm dt;
    memset(&dt, 0, sizeof(dt));

    dt.tm_year = date.year + 100; //tm_year: год - 1900
    dt.tm_mon = date.month - 1;   //tm_mon [0,11]
    dt.tm_mday = date.day;
    dt.tm_hour = time.hour;
    dt.tm_min = time.minute;
    dt.tm_sec = time.second;

    const int64_t tstamp = time_utc_from_local(&dt);
    timestamp_seq_interval(&seq, &date, &time,depth, interval);
    mu_assert_int_eq(tstamp, timestamp_seq_get(&seq, index));
}

MU_TEST_SUITE(suite_month)
{
    MU_RUN_TEST(test_month_12);
    MU_RUN_TEST(test_month_48);
    MU_RUN_TEST(test_month_cv);
}

MU_TEST_SUITE(suite_day)
{
    MU_RUN_TEST(test_day_365);
    MU_RUN_TEST(test_day_366);
    MU_RUN_TEST(test_day_trans);
    MU_RUN_TEST(test_day_cv);
}

MU_TEST_SUITE(suite_hour)
{
    MU_RUN_TEST(test_hour_16);
    MU_RUN_TEST(test_hour_32);
    MU_RUN_TEST(test_hour_64);
    MU_RUN_TEST(test_hour_cv);
}

MU_TEST_SUITE(suite_interval)
{
    MU_RUN_TEST(test_interval_5);
    MU_RUN_TEST(test_interval_cv);
}


int main()
{
    MU_RUN_SUITE(suite_month);
    MU_RUN_SUITE(suite_day);
    MU_RUN_SUITE(suite_hour);
    MU_RUN_SUITE(suite_interval);
    MU_REPORT();
    return mu_get_fails();
}

#ifdef __cplusplus
}
#endif
