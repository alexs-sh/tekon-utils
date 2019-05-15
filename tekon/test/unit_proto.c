/* Copyright (c) 2019
 * Alexander Shirokov
 * Schneider Electric
 * See LICENSE for details. */

#ifdef __cplusplus
extern "C" {
#endif

#include "test/minunit.h"
#include "tekon/time.h"

MU_TEST(test_unpack_time)
{
    const uint32_t data = 0x15141300;
    struct tekon_time tt;
    int result = tekon_time_unpack(&tt, &data, sizeof(data));
    mu_assert_int_eq(1, result);
    mu_assert_int_eq(15, tt.hour);
    mu_assert_int_eq(14, tt.minute);
    mu_assert_int_eq(13, tt.second);
}

MU_TEST(test_pack_time)
{
    struct tekon_time tt = {.hour = 15, .minute = 14, .second = 13};
    uint8_t buffer[4];
    const uint32_t check = 0x15141300;

    tekon_time_pack(&tt, buffer, sizeof(buffer));

    size_t i;
    const uint8_t * ptr = (const uint8_t *)&check;

    for(i = 0 ; i < sizeof(check); i++, ptr++) {
        mu_assert_int_eq(*ptr, buffer[i]);
    }
}

MU_TEST(test_unpack_date)
{
    const uint32_t data =  0x19042503;
    struct tekon_date td ;
    int result = tekon_date_unpack(&td, &data, sizeof(data));

    mu_assert_int_eq(1, result);
    mu_assert_int_eq(19, td.year);
    mu_assert_int_eq(4, td.month);
    mu_assert_int_eq(25, td.day);
    mu_assert_int_eq(3, td.dow);
}

MU_TEST(test_pack_date)
{
    struct tekon_date td = {.year = 19, .month = 4, .day = 25, .dow = 3};
    uint8_t buffer[4];
    const uint32_t check = 0x19042503;

    tekon_date_pack(&td, buffer, sizeof(buffer));

    size_t i;
    const uint8_t * ptr = (const uint8_t *)&check;

    for(i = 0 ; i < sizeof(check); i++) {
        mu_assert_int_eq(ptr[i], buffer[i]);
    }
}

MU_TEST_SUITE(suite_time)
{
    MU_RUN_TEST(test_unpack_time);
    MU_RUN_TEST(test_pack_time);
}

MU_TEST_SUITE(suite_date)
{
    MU_RUN_TEST(test_unpack_date);
    MU_RUN_TEST(test_pack_date);
}

int main()
{
    MU_RUN_SUITE(suite_time);
    MU_RUN_SUITE(suite_date);
    MU_REPORT();
    return mu_get_fails();
}

#ifdef __cplusplus
}
#endif
