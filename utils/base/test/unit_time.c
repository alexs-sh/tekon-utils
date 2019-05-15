/* Copyright (c) 2019
 * Alexander Shirokov
 * Schneider Electric
 * See LICENSE for details. */

#ifdef __cplusplus
extern "C" {
#endif

#include "test/minunit.h"
#include "utils/base/time.h"

#if defined(__unix__) || defined(__linux__)
// UNIX or LINUX
#include <time.h>
#elif defined(_WIN32) || defined(WIN32)
// WINDOWS
#include <sys/timeb.h>
#endif


MU_TEST(test_timestamp)
{
    int64_t utc = time_now_utc();
    int64_t local = time_now_local();

#if defined(__unix__) || defined(__linux__)
    // UNIX or LINUX
    struct tm ldt;
    time_t tmp = time(NULL);
    localtime_r(&tmp, &ldt);
    mu_assert_int_eq(utc + ldt.tm_gmtoff, local);

#elif defined(_WIN32) || defined(WIN32)
    // WINDOWS
    struct _timeb tb;
    _ftime( &tb );
    mu_assert_int_eq(utc - tb.timezone * 60, local);
#endif
}

MU_TEST(test_datetime)
{
    time_t utc = time_now_utc();
    struct tm ldt;

    int result = time_local_from_utc(utc, &ldt);
    mu_assert_int_eq(1, result);

    int64_t utc_from_ldt = time_utc_from_local(&ldt);
    mu_assert_int_eq(utc, utc_from_ldt);
}

MU_TEST_SUITE(suite_time)
{
    MU_RUN_TEST(test_timestamp);
    MU_RUN_TEST(test_datetime);
}

int main()
{
    MU_RUN_SUITE(suite_time);
    MU_REPORT();
    return mu_get_fails();
}

#ifdef __cplusplus
}
#endif
