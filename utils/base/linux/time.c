/* Copyright (c) 2019
 * Alexander Shirokov
 * Schneider Electric
 * See LICENSE for details. */

#ifdef __cplusplus
extern "C" {
#endif

#include "utils/base/time.h"
#include <assert.h>

int64_t time_now_utc()
{
    return time(NULL);
}

int64_t time_now_local()
{
    // Логика простая. Получаем вермя в UTC. Прибавляем сдвиг часового пояса в
    // секундах.
    time_t utc = time(NULL);
    struct tm ldt;

    localtime_r(&utc, &ldt);
    return utc + ldt.tm_gmtoff;
}

int32_t time_tzoffset()
{
    struct tm ldt;
    time_t utc = 0;
    localtime_r(&utc, &ldt);
    return ldt.tm_gmtoff;
}

int time_local_from_utc(int64_t utc, struct tm * local)
{
    assert(local);
    const time_t tutc = utc; //для корректной работы в 32-х битных системах
    return localtime_r(&tutc, local) != NULL;
}

int64_t time_utc_from_local(const struct tm * local)
{
    assert(local);
    struct tm tmp = *local;
    return mktime(&tmp);
}

#ifdef __cplusplus
}
#endif
