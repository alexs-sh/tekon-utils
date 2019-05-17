/* Copyright (c) 2019
 * Alexander Shirokov
 * Schneider Electric
 * See LICENSE for details. */

#ifdef __cplusplus
extern "C" {
#endif

#include "utils/sync/check.h"

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "utils/base/time.h"
#include "utils/base/tstamp.h"
#include "utils/base/string.h"

static int check_diff(int64_t newtime, int64_t devtime, int64_t adiff)
{
    if(newtime == TIME_INVALID)
        return -1;

    if(devtime == TIME_INVALID)
        return -2;

    const int64_t diff =  llabs(newtime - devtime);
    int result = diff <= adiff;

    if(!result)
        return -3;

    return 0;
}

static int check_minutes(int64_t newtime, int64_t devtime, int64_t adiff)
{
    if(newtime == TIME_INVALID)
        return -1;

    if(devtime == TIME_INVALID)
        return -2;

    int result = newtime / adiff == devtime / adiff;

    if(!result)
        return -3;

    return 0;
}

static int check_indexes(const struct tm * newtime, const struct tm * devtime)
{
    assert(newtime);
    assert(devtime);

    //Последовательно проверяем индексы и убеждаемся, что они не будут сдвинуты
    //при перестановке времени
    struct tekon_date devd;
    struct tekon_time devt;

    struct tekon_date newd;
    struct tekon_time newt;

    tekon_date_from_local(&devd, devtime);
    tekon_time_from_local(&devt, devtime);

    tekon_date_from_local(&newd, newtime);
    tekon_time_from_local(&newt, newtime);

    int devi = tekon_month_index(devd.year, devd.month, 12);
    int newi = tekon_month_index(newd.year, newd.month, 12);

    if(devi != newi)
        return -1;

    devi = tekon_day_index(devd.year, devd.month, devd.day);
    newi = tekon_day_index(newd.year, newd.month, newd.day);

    if(devi != newi)
        return -2;

    devi = tekon_hour_index(devd.year, devd.month, devd.day, devt.hour, 1536);
    newi = tekon_hour_index(newd.year, newd.month, newd.day, newt.hour, 1536);

    if(devi != newi)
        return -3;

    devi = tekon_interval_index(devd.year, devd.month, devd.day, devt.hour, devt.minute, 1440, 5);
    newi = tekon_interval_index(newd.year, newd.month, newd.day, newt.hour, newt.minute, 1440, 5);

    if(devi != newi)
        return -4;

    return 0;
}

void checks_init(struct checks * self)
{
    assert(self);

    memset(self, 0, sizeof(*self));
    self->avail = TIME_CHECK_DIFF | TIME_CHECK_MINUTES;
    self->diff = 30;
    self->minutes = 1;
}

int checks_from_string(struct checks * self, const char * str)
{
    assert(self);

    if(string_is_term(str))
        return 0;

    char buffer[256] = {0};
    char * saveptr = NULL;
    char * token = NULL;

    const size_t buflen = sizeof(buffer);
    const size_t arglen = strnlen(str, buflen);

    if(arglen == buflen)
        return 0;

    strncpy(buffer, str, buflen - 1);
    char * ptr = buffer;

    struct checks result;
    memset(&result, 0, sizeof(result));

    while((token = strtok_r(ptr, " ", &saveptr)) != NULL) {
        ptr = saveptr;
        if(strncmp(token, "none", 4) == 0) {
            if(result.avail != TIME_CHECK_NONE)
                return 0;
            else
                break;
        } else if(strncmp(token, "difference", 10) == 0) {
            char * value = NULL;
            strtok_r(token, ":", &value);
            result.diff = atoi(value);

            if(result.diff <= 0)
                return 0;

            result.avail |= TIME_CHECK_DIFF;
        } else if(strncmp(token, "minutes", 7) == 0) {
            char * value = NULL;
            strtok_r(token, ":", &value);
            result.minutes = atoi(value);

            if(result.minutes <= 0)
                return 0;

            result.avail |= TIME_CHECK_MINUTES;
        } else if(strncmp(token, "indexes", 7) == 0) {
            result.avail |= TIME_CHECK_INDEX;
        } else {
            return 0;
        }

        if(strncmp(token, "indexes", 7) == 0) {
            result.avail |= TIME_CHECK_INDEX;
        }
    }

    memcpy(self, &result, sizeof(result));
    return 1;
}

int checks_run(struct checks * self, const struct tm * newtime, const struct tm * devtime)
{
    assert(self);
    assert(newtime);
    assert(devtime);

    int result = 0;
    int64_t newt_utc = time_utc_from_local(newtime);
    int64_t dev_utc = time_utc_from_local(devtime);

    if(self->avail & TIME_CHECK_DIFF) {
        result = check_diff(newt_utc, dev_utc, self->diff);

        if(result != 0) {
            self->fail |= TIME_CHECK_DIFF;
            snprintf(self->message, sizeof(self->message),
                     "difference check failed");
            return 0;
        }
    }
    if(self->avail & TIME_CHECK_MINUTES) {
        result = check_minutes(newt_utc, dev_utc, self->minutes * 60);
        if(result != 0) {
            self->fail |= TIME_CHECK_MINUTES;
            snprintf(self->message, sizeof(self->message),
                     "minutes check failed");
            return 0;
        }
    }
    if(self->avail & TIME_CHECK_INDEX) {
        result = check_indexes(newtime, devtime);
        if(result != 0) {
            self->fail |= TIME_CHECK_INDEX;
            snprintf(self->message, sizeof(self->message),
                     "indexes check failed");
            return 0;
        }
    }
    return 1;
}


#ifdef __cplusplus
}
#endif
