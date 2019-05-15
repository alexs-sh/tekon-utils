/* Copyright (c) 2019
 * Alexander Shirokov
 * Schneider Electric
 * See LICENSE for details. */

#ifdef __cplusplus
extern "C" {
#endif

#include "test/minunit.h"
#include "utils/sync/check.h"
#include "utils/base/time.h"

MU_TEST(test_check_from_string)
{
    struct checks checks;
    int result = checks_from_string(&checks, "none");
    mu_assert_int_eq(1, result);
    mu_assert_int_eq(TIME_CHECK_NONE, checks.avail);

    result = checks_from_string(&checks, "difference:100");
    mu_assert_int_eq(1, result);
    mu_assert_int_eq(TIME_CHECK_DIFF, checks.avail);
    mu_assert_int_eq(100, checks.diff);

    result = checks_from_string(&checks, "minutes:5");
    mu_assert_int_eq(1, result);
    mu_assert_int_eq(TIME_CHECK_MINUTES, checks.avail);
    mu_assert_int_eq(5, checks.minutes);

    result = checks_from_string(&checks, "indexes");
    mu_assert_int_eq(1, result);
    mu_assert_int_eq(TIME_CHECK_INDEX, checks.avail);

    result = checks_from_string(&checks, "difference:12 minutes:3 indexes");
    mu_assert_int_eq(1, result);
    mu_assert(checks.avail & TIME_CHECK_DIFF, "TIME_CHECK_DIFF is missing");
    mu_assert(checks.avail & TIME_CHECK_MINUTES, "TIME_CHECK_MINUTES is missing");
    mu_assert(checks.avail & TIME_CHECK_INDEX, "TIME_CHECK_INDEX is missing");

    mu_assert_int_eq(12, checks.diff);
    mu_assert_int_eq(3, checks.minutes);

}

MU_TEST(test_check_from_string_inv)
{

    char long_string[1024];

    struct checks checks;
    int result = checks_from_string(&checks, NULL);
    mu_assert_int_eq(0, result);

    result = checks_from_string(&checks, "");
    mu_assert_int_eq(0, result);

    memset(long_string, 'a', sizeof(long_string));

    result = checks_from_string(&checks, long_string);
    mu_assert_int_eq(0, result);

    result = checks_from_string(&checks, "difference:100 none");
    mu_assert_int_eq(0, result);


    result = checks_from_string(&checks, "difference");
    mu_assert_int_eq(0, result);

    result = checks_from_string(&checks, "difference");
    mu_assert_int_eq(0, result);

    result = checks_from_string(&checks, "minutes");
    mu_assert_int_eq(0, result);

    result = checks_from_string(&checks, "difference:-5");
    mu_assert_int_eq(0, result);

    result = checks_from_string(&checks, "minutes:-5");
    mu_assert_int_eq(0, result);

    result = checks_from_string(&checks, "unknown");
    mu_assert_int_eq(0, result);

}

MU_TEST(test_checks_run_diff)
{
    struct checks checks;
    checks_from_string(&checks, "difference:100");

    struct tm devtime;
    struct tm newtime;

    time_local_from_utc(1000, &devtime);
    time_local_from_utc(1010, &newtime);

    int result = checks_run(&checks, &newtime, &devtime);
    mu_assert_int_eq(1, result);

    time_local_from_utc(1000, &devtime);
    time_local_from_utc(1111, &newtime);

    result = checks_run(&checks, &newtime, &devtime);
    mu_assert_int_eq(0, result);

    mu_assert(checks.fail & TIME_CHECK_DIFF, "");

}

MU_TEST(test_checks_run_minutes_1)
{
    struct checks checks;
    checks_from_string(&checks, "minutes:1");

    struct tm devtime;
    struct tm newtime;

    int result;

    memset(&devtime, 0, sizeof(devtime));
    memset(&newtime, 0, sizeof(newtime));

    devtime.tm_year = 119;
    devtime.tm_mon = 5;
    devtime.tm_mday = 1;
    devtime.tm_hour = 1;
    devtime.tm_min = 0;
    devtime.tm_sec = 0;

    memcpy(&newtime, &devtime, sizeof(devtime));

    size_t i;
    for(i = 0; i < 60; i++) {
        result = checks_run(&checks, &newtime, &devtime);
        mu_assert_int_eq(1, result);
        newtime.tm_sec = i;
    }

    memcpy(&newtime, &devtime, sizeof(devtime));
    newtime.tm_min = 1;

    result = checks_run(&checks, &newtime, &devtime);
    mu_assert_int_eq(0, result);
    mu_assert_int_eq(TIME_CHECK_MINUTES, checks.fail);
}

MU_TEST(test_checks_run_minutes_2)
{
    struct checks checks;
    checks_from_string(&checks, "minutes:2");

    struct tm devtime;
    struct tm newtime;

    int result;

    memset(&devtime, 0, sizeof(devtime));
    memset(&newtime, 0, sizeof(newtime));

    devtime.tm_year = 119;
    devtime.tm_mon = 5;
    devtime.tm_mday = 1;
    devtime.tm_hour = 1;
    devtime.tm_min = 0;
    devtime.tm_sec = 0;

    memcpy(&newtime, &devtime, sizeof(devtime));

    size_t i;
    size_t m;
    for(m = 0; m < 10; m++) {

        for(i = 0; i < 60; i++) {
            result = checks_run(&checks, &newtime, &devtime);
            if(m < 2) {
                mu_assert_int_eq(1, result);
            } else {
                mu_assert_int_eq(0, result);
                mu_assert_int_eq(TIME_CHECK_MINUTES, checks.fail);
            }
            newtime.tm_sec = i;
        }
        newtime.tm_min++;
    }
}

MU_TEST(test_checks_run_indexes)
{
    struct checks checks;
    checks_from_string(&checks, "indexes");

    struct tm devtime;
    struct tm newtime;

    int result;

    memset(&devtime, 0, sizeof(devtime));
    memset(&newtime, 0, sizeof(newtime));

    devtime.tm_year = 119;
    devtime.tm_mon = 5;
    devtime.tm_mday = 1;
    devtime.tm_hour = 1;
    devtime.tm_min = 0;
    devtime.tm_sec = 0;

    memcpy(&newtime, &devtime, sizeof(devtime));

    result = checks_run(&checks, &newtime, &devtime);
    mu_assert_int_eq(1, result);

    // Сдвиг часового индекса
    newtime.tm_hour++;
    result = checks_run(&checks, &newtime, &devtime);
    mu_assert_int_eq(0, result);
    mu_assert_int_eq(TIME_CHECK_INDEX, checks.fail);
    checks.fail = TIME_CHECK_NONE;

    // Сдвиг суточного индекса
    memcpy(&newtime, &devtime, sizeof(devtime));
    newtime.tm_mday++;
    result = checks_run(&checks, &newtime, &devtime);
    mu_assert_int_eq(0, result);
    mu_assert_int_eq(TIME_CHECK_INDEX, checks.fail);
    checks.fail = TIME_CHECK_NONE;

    // Сдвиг месячного индекса
    memcpy(&newtime, &devtime, sizeof(devtime));
    newtime.tm_mon++;
    result = checks_run(&checks, &newtime, &devtime);
    mu_assert_int_eq(0, result);
    mu_assert_int_eq(TIME_CHECK_INDEX, checks.fail);
    checks.fail = TIME_CHECK_NONE;

    // Сдвиг интервального индекса (5 мин)
    memcpy(&newtime, &devtime, sizeof(devtime));
    newtime.tm_min += 7;
    result = checks_run(&checks, &newtime, &devtime);
    mu_assert_int_eq(0, result);
    mu_assert_int_eq(TIME_CHECK_INDEX, checks.fail);
    checks.fail = TIME_CHECK_NONE;

}

MU_TEST_SUITE(suite_checks)
{
    MU_RUN_TEST(test_check_from_string);
    MU_RUN_TEST(test_checks_run_diff);
    MU_RUN_TEST(test_checks_run_minutes_1);
    MU_RUN_TEST(test_checks_run_minutes_2);
    MU_RUN_TEST(test_checks_run_indexes);
}

MU_TEST_SUITE(suite_checks_inv)
{
    MU_RUN_TEST(test_check_from_string_inv);
}

int main()
{
    MU_RUN_SUITE(suite_checks);
    MU_RUN_SUITE(suite_checks_inv);
    MU_REPORT();
    return mu_get_fails();
}

#ifdef __cplusplus
}
#endif
