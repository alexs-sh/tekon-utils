/* Copyright (c) 2019
 * Alexander Shirokov
 * Schneider Electric
 * See LICENSE for details. */

#ifdef __cplusplus
extern "C" {
#endif

#include "test/minunit.h"
#include "utils/base/link.h"
#include <errno.h>

MU_TEST(test_udp)
{

    struct link link;
    char buffer[64] = {0};
    int result = link_init_udp(&link, "127.0.0.1", 8888, 100);
    mu_assert_int_eq(0, result);
    mu_assert_int_eq(link.timeout, 100);
    mu_assert_int_eq(link.type, LINK_UDP);

    result = link_up(&link);
    mu_assert_int_eq(0, result);

    result = link_up(&link);
    mu_assert_int_eq(-EISCONN, result);

    result = link_send(&link, buffer, sizeof(buffer));
    mu_assert_int_eq(sizeof(buffer), result);

    result = link_recv(&link, buffer, sizeof(buffer));
    mu_assert(result <= 0, "result should be less or equal 0");

    link_down(&link);
}

MU_TEST(test_tcp)
{

    struct link link;

    int result = link_init_tcp(&link, "127.0.0.1", 8888, 100);
    mu_assert_int_eq(0, result);
    mu_assert_int_eq(link.timeout, 100);
    mu_assert_int_eq(link.type, LINK_TCP);
}


MU_TEST(test_invalid_address)
{
    struct link link;
    int result = link_init_udp(&link, "bbzzzshit$", 8888, 100);
    mu_assert_int_eq(-EINVAL, result);
}


MU_TEST(test_invalid_usage)
{
    struct link link;
    char buffer[64] = {0};
    link_init_udp(&link, "127.0.0.1", 8888, 100);

    int result = link_send(&link, buffer, sizeof(buffer));
    mu_assert_int_eq(-EBADF, result);

    result = link_recv(&link, buffer, sizeof(buffer));
    mu_assert_int_eq(-EBADF, result);

    link_down(&link);

}

MU_TEST_SUITE(suite_base)
{
    MU_RUN_TEST(test_udp);
    MU_RUN_TEST(test_tcp);
}

MU_TEST_SUITE(suite_invalid)
{
    MU_RUN_TEST(test_invalid_address);
    MU_RUN_TEST(test_invalid_usage);
}

int main()
{
    MU_RUN_SUITE(suite_base);
    MU_RUN_SUITE(suite_invalid);
    MU_REPORT();
    return mu_get_fails();
}

#ifdef __cplusplus
}
#endif
