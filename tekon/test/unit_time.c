/* Copyright (c) 2019
 * Alexander Shirokov
 * Schneider Electric
 * See LICENSE for details. */

#ifdef __cplusplus
extern "C" {
#endif

#include "test/minunit.h"
#include "tekon/time.h"

MU_TEST(test_month_idx_12)
{

    /* контрольные значения */
    mu_assert_int_eq(0x08, tekon_month_index(16, 9, 12));
    mu_assert_int_eq(0x06, tekon_month_index(17, 7, 12));

    /* перебор */
    size_t i;
    for(i = 0; i < 12; i++) {
        mu_assert_int_eq(i, tekon_month_index(19, i + 1, 12));
    }
}

MU_TEST(test_month_idx_48)
{
    mu_assert_int_eq(42, tekon_month_index(19, 7, 48));
}

MU_TEST(test_month_idx_inv)
{

    mu_assert_int_eq(TEKON_INVALID_ARCH_INDEX, tekon_month_index(19, 0, 12));
    mu_assert_int_eq(TEKON_INVALID_ARCH_INDEX, tekon_month_index(100, 1, 12));
    mu_assert_int_eq(TEKON_INVALID_ARCH_INDEX, tekon_month_index(19, 1, 5));
    mu_assert_int_eq(TEKON_INVALID_ARCH_INDEX, tekon_month_index(19, 1, 15));
    mu_assert_int_eq(TEKON_INVALID_ARCH_INDEX, tekon_month_index(19, 1, 55));
}

MU_TEST(test_day)
{

    mu_assert_int_eq(0x0144, tekon_day_index(16, 11, 20));
    mu_assert_int_eq(0x015a, tekon_day_index(16, 12, 12));
    mu_assert_int_eq(0x53, tekon_day_index(17, 3, 25));
    mu_assert_int_eq(0x61, tekon_day_index(17, 4, 8));
    mu_assert_int_eq(0xce, tekon_day_index(17, 7, 26));
    mu_assert_int_eq(0xff, tekon_day_index(17, 9, 13));

}

MU_TEST(test_hour)
{
    mu_assert_int_eq(0x82, tekon_hour_index(17,   9, 17, 10, 1536));
    mu_assert_int_eq(0x0115, tekon_hour_index(17,   7, 21, 13,1536));
    mu_assert_int_eq(0x0416, tekon_hour_index(18,   5, 5, 14, 1536));
    mu_assert_int_eq(0x0446, tekon_hour_index(18,   5,7, 14, 1536));
    mu_assert_int_eq(0x0447, tekon_hour_index(18,   5,7, 15, 1536));
    mu_assert_int_eq(0x0448, tekon_hour_index(18,   5,7, 16, 1536));
    mu_assert_int_eq(0x023f, tekon_hour_index(18,   4,15, 23, 768));
    mu_assert_int_eq(0x22, tekon_hour_index(18,   4,25, 10, 384));
    mu_assert_int_eq(0x0116, tekon_hour_index(18,   5,5, 14, 384));
}

MU_TEST(test_interval)
{

    mu_assert_int_eq(0x0564, tekon_interval_index(18, 5, 5, 19, 0, 1440, 5));
    mu_assert_int_eq(0x0219, tekon_interval_index(18, 5, 7, 20, 45, 1440, 5));
}

MU_TEST(test_date_validate)
{
    mu_assert_int_eq(1, tekon_date_is_valid(&(struct tekon_date) {
        1,1,1,0
    }));
    mu_assert_int_eq(1, tekon_date_is_valid(&(struct tekon_date) {
        19,2,28,0
    }));


    mu_assert_int_eq(0, tekon_date_is_valid(&(struct tekon_date) {
        19,2,29,0
    }));
    mu_assert_int_eq(0, tekon_date_is_valid(&(struct tekon_date) {
        119,1,1,0
    }));
    mu_assert_int_eq(0, tekon_date_is_valid(&(struct tekon_date) {
        19,0,1,0
    }));
    mu_assert_int_eq(0, tekon_date_is_valid(&(struct tekon_date) {
        19,1,0,0
    }));
    mu_assert_int_eq(0, tekon_date_is_valid(&(struct tekon_date) {
        19,13,1,0
    }));
    mu_assert_int_eq(0, tekon_date_is_valid(&(struct tekon_date) {
        19,11,31,0
    }));
}

MU_TEST(test_time_validate)
{
    mu_assert_int_eq(1, tekon_time_is_valid(&(struct tekon_time) {
        0,0,0
    }));
    mu_assert_int_eq(1, tekon_time_is_valid(&(struct tekon_time) {
        1,1,1
    }));
    mu_assert_int_eq(1, tekon_time_is_valid(&(struct tekon_time) {
        23,59,59
    }));

    mu_assert_int_eq(0, tekon_time_is_valid(&(struct tekon_time) {
        24,59,59
    }));
    mu_assert_int_eq(0, tekon_time_is_valid(&(struct tekon_time) {
        23,60,59
    }));
    mu_assert_int_eq(0, tekon_time_is_valid(&(struct tekon_time) {
        23,59,60
    }));
}
MU_TEST_SUITE(suite_indexes)
{
    MU_RUN_TEST(test_month_idx_12);
    MU_RUN_TEST(test_month_idx_48);
    MU_RUN_TEST(test_day);
    MU_RUN_TEST(test_hour);
    MU_RUN_TEST(test_interval);
}

MU_TEST_SUITE(suite_indexes_inv)
{
    MU_RUN_TEST(test_month_idx_inv);
}

MU_TEST_SUITE(suite_date_validate)
{
    MU_RUN_TEST(test_date_validate);
}

MU_TEST_SUITE(suite_time_validate)
{
    MU_RUN_TEST(test_time_validate);
}
int main()
{
    MU_RUN_SUITE(suite_indexes);
    MU_RUN_SUITE(suite_indexes_inv);
    MU_RUN_SUITE(suite_date_validate);
    MU_RUN_SUITE(suite_time_validate);
    MU_REPORT();
    return mu_get_fails();
}

#ifdef __cplusplus
}
#endif
