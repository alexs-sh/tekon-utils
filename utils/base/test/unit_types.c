/* Copyright (c) 2019
 * Alexander Shirokov
 * Schneider Electric
 * See LICENSE for details. */

#ifdef __cplusplus
extern "C" {
#endif

#include "test/minunit.h"
#include "utils/base/types.h"

MU_TEST(test_netaddr_udp)
{
    struct netaddr address;
    const char * str = "udp:192.168.1.2:51960@1";
    int result = netaddr_from_string(&address, str);
    mu_assert_int_eq(1, result);
    mu_assert_int_eq(LINK_UDP, address.type);
    mu_assert_int_eq(51960, address.port);
    mu_assert_int_eq(1, address.gateway);
    mu_assert_string_eq("192.168.1.2", address.ip);
}

MU_TEST(test_netaddr_tcp)
{
    struct netaddr address;
    const char * str = "tcp:10.0.0.5:8888@100";
    int result = netaddr_from_string(&address, str);
    mu_assert_int_eq(1, result);
    mu_assert_int_eq(LINK_TCP, address.type);
    mu_assert_int_eq(8888, address.port);
    mu_assert_int_eq(100, address.gateway);
    mu_assert_string_eq("10.0.0.5", address.ip);
}

MU_TEST(test_netaddr_trim)
{
    struct netaddr address;
    const char * str = "    udp:192.168.1.2:51960@1";
    int result = netaddr_from_string(&address, str);
    mu_assert_int_eq(1, result);
    mu_assert_int_eq(LINK_UDP, address.type);
    mu_assert_int_eq(51960, address.port);
    mu_assert_int_eq(1, address.gateway);
    mu_assert_string_eq("192.168.1.2", address.ip);
}

MU_TEST(test_netaddr_inv)
{
    struct netaddr address;
    int result = netaddr_from_string(&address, "zdp:192.168.1.2:51960@1");
    mu_assert_int_eq(0, result);

    result = netaddr_from_string(&address, "udp:1922.1.168.2:51960@1");
    mu_assert_int_eq(0, result);

    result = netaddr_from_string(&address, "192.1.2:51960@1");
    mu_assert_int_eq(0, result);

    result = netaddr_from_string(&address, "udp:192.1.168.2:519600@1");
    mu_assert_int_eq(0, result);

    result = netaddr_from_string(&address, "udp:192.1.168.2:-51960@1");
    mu_assert_int_eq(0, result);

    result = netaddr_from_string(&address, "udp:sldkjfsdkjfasddlfgkjdsfkgjsldfgjlsdfkjgldsfjgldsfgjdslfgjdslfgjldfkjgsdlfgjdslfgjdlfkg:51960@1");
    mu_assert_int_eq(0, result);

    result = netaddr_from_string(&address, "udp:192.1.168.2:6666666@1");
    mu_assert_int_eq(0, result);

    result = netaddr_from_string(&address, "udp:192.1.168.2:51960@-1");
    mu_assert_int_eq(0, result);

    result = netaddr_from_string(&address, "udp:192.1.168.2:51960@1000");
    mu_assert_int_eq(0, result);

    result = netaddr_from_string(&address, "udp:192.1.168.2:51960@");
    mu_assert_int_eq(0, result);

    result = netaddr_from_string(&address, "udp:51960@1");
    mu_assert_int_eq(0, result);

    result = netaddr_from_string(&address, "");
    mu_assert_int_eq(0, result);

    result = netaddr_from_string(&address, NULL);
    mu_assert_int_eq(0, result);

}

MU_TEST(test_paraddr_hex)
{
    struct paraddr address;
    const char * str = "1:0x100:0:F";
    int result = paraddr_from_string(&address, str);
    mu_assert_int_eq(1, result);
    mu_assert_int_eq(TEKON_PARAM_F32, address.type);
    mu_assert_int_eq(1, address.device);
    mu_assert_int_eq(0x100, address.address);
    mu_assert_int_eq(0, address.index);
    mu_assert_int_eq(1, address.hex);
}

MU_TEST(test_paraddr_dec)
{
    struct paraddr address;
    const char * str = "13:100:1:U";
    int result = paraddr_from_string(&address, str);
    mu_assert_int_eq(1, result);
    mu_assert_int_eq(TEKON_PARAM_U32, address.type);
    mu_assert_int_eq(13, address.device);
    mu_assert_int_eq(100, address.address);
    mu_assert_int_eq(1, address.index);
    mu_assert_int_eq(0, address.hex);
}

MU_TEST(test_paraddr_types)
{
    struct paraddr address;
    int result = paraddr_from_string(&address, "1:100:0:F");
    mu_assert_int_eq(1, result);
    mu_assert_int_eq(TEKON_PARAM_F32, address.type);

    result = paraddr_from_string(&address, "1:100:0:f");
    mu_assert_int_eq(1, result);
    mu_assert_int_eq(TEKON_PARAM_F32, address.type);

    result = paraddr_from_string(&address, "1:100:0:U");
    mu_assert_int_eq(1, result);
    mu_assert_int_eq(TEKON_PARAM_U32, address.type);

    result = paraddr_from_string(&address, "1:100:0:u");
    mu_assert_int_eq(1, result);
    mu_assert_int_eq(TEKON_PARAM_U32, address.type);

    result = paraddr_from_string(&address, "1:100:0:R");
    mu_assert_int_eq(1, result);
    mu_assert_int_eq(TEKON_PARAM_RAW, address.type);

    result = paraddr_from_string(&address, "1:100:0:r");
    mu_assert_int_eq(1, result);
    mu_assert_int_eq(TEKON_PARAM_RAW, address.type);

    result = paraddr_from_string(&address, "1:100:0:B");
    mu_assert_int_eq(1, result);
    mu_assert_int_eq(TEKON_PARAM_BOOL, address.type);


    result = paraddr_from_string(&address, "1:100:0:b");
    mu_assert_int_eq(1, result);
    mu_assert_int_eq(TEKON_PARAM_BOOL, address.type);

    result = paraddr_from_string(&address, "1:100:0:T");
    mu_assert_int_eq(1, result);
    mu_assert_int_eq(TEKON_PARAM_TIME, address.type);

    result = paraddr_from_string(&address, "1:100:0:t");
    mu_assert_int_eq(1, result);
    mu_assert_int_eq(TEKON_PARAM_TIME, address.type);

    result = paraddr_from_string(&address, "1:100:0:D");
    mu_assert_int_eq(1, result);
    mu_assert_int_eq(TEKON_PARAM_DATE, address.type);

    result = paraddr_from_string(&address, "1:100:0:d");
    mu_assert_int_eq(1, result);
    mu_assert_int_eq(TEKON_PARAM_DATE, address.type);

    result = paraddr_from_string(&address, "1:100:0:h");
    mu_assert_int_eq(1, result);
    mu_assert_int_eq(TEKON_PARAM_HEX, address.type);


}


MU_TEST(test_paraddr_inv)
{
    struct paraddr address;
    int result = paraddr_from_string(&address, "1000:0x111:0:F");
    mu_assert_int_eq(0, result);

    result = paraddr_from_string(&address, "100::F");
    mu_assert_int_eq(0, result);

    result = paraddr_from_string(&address, ":0x111:0:F");
    mu_assert_int_eq(0, result);

    result = paraddr_from_string(&address, "100:0x111:0:K");
    mu_assert_int_eq(0, result);

    result = paraddr_from_string(&address, "100:0x111:");
    mu_assert_int_eq(0, result);

    result = paraddr_from_string(&address, "100:0:K");
    mu_assert_int_eq(0, result);

    result = paraddr_from_string(&address, "");
    mu_assert_int_eq(0, result);

    result = paraddr_from_string(&address, NULL);
    mu_assert_int_eq(0, result);
}

MU_TEST(test_archaddr_dec)
{
    struct paraddr address;
    const char * str = "13:100:0:1440:U";
    int result = archaddr_from_string(&address, str);
    mu_assert_int_eq(1, result);
    mu_assert_int_eq(TEKON_PARAM_U32, address.type);
    mu_assert_int_eq(13, address.device);
    mu_assert_int_eq(100, address.address);
    mu_assert_int_eq(0, address.index);
    mu_assert_int_eq(1440, address.count);
}

MU_TEST(test_archaddr_hex)
{
    struct paraddr address;
    const char * str = "22:0x100:1440:8543:F";
    int result = archaddr_from_string(&address, str);
    mu_assert_int_eq(1, result);
    mu_assert_int_eq(TEKON_PARAM_F32, address.type);
    mu_assert_int_eq(22, address.device);
    mu_assert_int_eq(0x100, address.address);
    mu_assert_int_eq(1440, address.index);
    mu_assert_int_eq(8543, address.count);
}

MU_TEST(test_dtaddr)
{
    //3:0xF017:0xF018
    struct dtaddr cfg;
    int result = dtaddr_from_string(&cfg, "3:0xF017:0xF018");
    mu_assert_int_eq(1,result);
    mu_assert_int_eq(3,cfg.device);
    mu_assert_int_eq(0xF017,cfg.date);
    mu_assert_int_eq(0xF018,cfg.time);
}

MU_TEST(test_intcfg)
{
    //3:0xF017:0xF018
    struct intcfg cfg;
    int result = intcfg_from_string(&cfg, "m:12");
    mu_assert_int_eq(1,result);
    mu_assert_int_eq('m',cfg.type);
    mu_assert_int_eq(12,cfg.depth);

    result = intcfg_from_string(&cfg, "d");
    mu_assert_int_eq(1,result);
    mu_assert_int_eq('d',cfg.type);

    result = intcfg_from_string(&cfg, "h:384");
    mu_assert_int_eq(1,result);
    mu_assert_int_eq('h',cfg.type);
    mu_assert_int_eq(384,cfg.depth);

    result = intcfg_from_string(&cfg, "i:1440:5");
    mu_assert_int_eq(1,result);
    mu_assert_int_eq('i',cfg.type);
    mu_assert_int_eq(1440,cfg.depth);
    mu_assert_int_eq(5,cfg.interval);
}

MU_TEST_SUITE(suite_netaddr)
{
    MU_RUN_TEST(test_netaddr_udp);
    MU_RUN_TEST(test_netaddr_tcp);
    MU_RUN_TEST(test_netaddr_trim);
}

MU_TEST_SUITE(suite_netaddr_invalid)
{
    MU_RUN_TEST(test_netaddr_inv);
}

MU_TEST_SUITE(suite_paraddr)
{
    MU_RUN_TEST(test_paraddr_hex);
    MU_RUN_TEST(test_paraddr_dec);
    MU_RUN_TEST(test_paraddr_types);
}

MU_TEST_SUITE(suite_paraddr_invalid)
{
    MU_RUN_TEST(test_paraddr_inv);
}

MU_TEST_SUITE(suite_archaddr)
{
    MU_RUN_TEST(test_archaddr_hex);
    MU_RUN_TEST(test_archaddr_dec);
}

MU_TEST_SUITE(suite_dtaddr)
{
    MU_RUN_TEST(test_dtaddr);
}

MU_TEST_SUITE(suite_intcfg)
{
    MU_RUN_TEST(test_intcfg);
}

int main()
{
    MU_RUN_SUITE(suite_netaddr);
    MU_RUN_SUITE(suite_netaddr_invalid);
    MU_RUN_SUITE(suite_paraddr);
    MU_RUN_SUITE(suite_paraddr_invalid);
    MU_RUN_SUITE(suite_archaddr);
    MU_RUN_SUITE(suite_dtaddr);
    MU_RUN_SUITE(suite_intcfg);
    MU_REPORT();
    return mu_get_fails();
}

#ifdef __cplusplus
}
#endif

