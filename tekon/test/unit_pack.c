/* Copyright (c) 2019
 * Alexander Shirokov
 * Schneider Electric
 * See LICENSE for details. */

#ifdef __cplusplus
extern "C" {
#endif

#include "test/minunit.h"
#include "tekon/pack.h"

MU_TEST(test_pack_nums)
{

    uint8_t buffer[1024] = {0};

    struct message message;
    tekon_req_11(&message, 2, 3, 0x8003);

    size_t i;
    for(i = 0; i < 255; i++) {
        int result = tekon_req_pack(buffer, sizeof(buffer), &message, i);
        if(i <= 15) {
            mu_assert_int_eq(9, result);
            mu_assert_int_eq(0x40 | i, buffer[1]);
        } else {
            mu_assert_int_eq(0, result);
        }
    }
}

MU_TEST(test_pack_readem_11)
{

    uint8_t buffer[1024] = {0};
    const uint8_t control_msg[] = {0x10, 0x41, 0x02, 0x11, 0x03,
                                   0x03, 0x80, 0xDA, 0x16
                                  };

    struct message message;
    tekon_req_11(&message, 2, 3, 0x8003);
    int result = tekon_req_pack(buffer, sizeof(buffer), &message, 1);

    mu_assert_int_eq(sizeof(control_msg), result);
    mu_assert_int_eq(TEKON_DIR_OUT, message.dir);
    mu_assert_int_eq(TEKON_MSG_READEM_PAR_11, message.type);

    for (size_t i = 0; i < sizeof(control_msg); i++) {
        mu_assert_int_eq(buffer[i], control_msg[i]);
    }
}


MU_TEST(test_pack_readem_11_inv_buffer)
{

    uint8_t buffer[4] = {0};

    struct message message;
    tekon_req_11(&message, 2, 3, 0x8003);
    int result = tekon_req_pack(buffer, sizeof(buffer), &message, 1);

    mu_assert_int_eq(0, result);
}

MU_TEST(test_pack_writem_14_passwd)
{

    uint8_t buffer[1024] = {0};
    const uint8_t gateway = 0x9;
    const uint8_t passwd[8] = {0x07, 0x3, 0x05, 0x02, 0x00, 0x00, 0x00, 0x01};
    const uint8_t control_msg[] = {0x68,0x0b,0x0b,0x68,0x42,0x09,
                                   0x14,0x07,0x03,0x05,0x02,
                                   0x00,0x00,0x00,0x01,
                                   0x71,0x16
                                  };

    struct message message;
    tekon_req_14(&message, gateway, passwd, sizeof(passwd));
    int result = tekon_req_pack(buffer, sizeof(buffer), &message, 2);

    mu_assert_int_eq(sizeof(control_msg), result);
    mu_assert_int_eq(TEKON_DIR_OUT, message.dir);
    mu_assert_int_eq(TEKON_MSG_WRITEM_PAR_14, message.type);

    for (size_t i = 0; i < sizeof(control_msg); i++) {
        mu_assert_int_eq(buffer[i], control_msg[i]);
    }
}

MU_TEST(test_pack_writem_14_write_reg)
{
    uint8_t buffer[1024] = {0};
    const uint8_t gateway = 0x9;
    const uint8_t cmd[9] = {0x08,0x03,0x03,0x18,0xf0,0x00,0x00,0x10,0x12};
    const uint8_t control_msg[] = {
        0x68,0x0c,0x0c,0x68,0x44,0x09,
        0x14,0x08,0x03,0x03,0x18,0xf0,0x00,0x00,0x10,0x12,0x99,0x16
    };

    struct message message;
    tekon_req_14(&message, gateway, cmd, sizeof(cmd));
    int result = tekon_req_pack(buffer, sizeof(buffer), &message, 4);

    mu_assert_int_eq(sizeof(control_msg), result);
    mu_assert_int_eq(TEKON_DIR_OUT, message.dir);
    mu_assert_int_eq(TEKON_MSG_WRITEM_PAR_14, message.type);

    for (size_t i = 0; i < sizeof(control_msg); i++) {
        mu_assert_int_eq(buffer[i], control_msg[i]);
    }
}

MU_TEST(test_pack_readem_19)
{

    uint8_t buffer[1024] = {0};
    const uint8_t control_msg[] = {0x68, 0x09, 0x09, 0x68, 0x41, 0x02, 0x19, 0x03,
                                   0xa9, 0x80, 0x03, 0x00, 0x09, 0x94, 0x16
                                  };
    struct message message;

    int result = tekon_req_19(&message, 2, 3, 0x80a9, 3, 9);
    mu_assert_int_eq(1, result);

    result = tekon_req_pack(buffer, sizeof(buffer), &message, 1);
    mu_assert_int_eq(sizeof(control_msg), result);

    for (size_t i = 0; i < sizeof(control_msg); i++) {
        mu_assert_int_eq(buffer[i], control_msg[i]);
    }
}

MU_TEST(test_pack_readem_list_1c_inv_buffer)
{
    uint8_t buffer[8];
    const uint8_t devices[8] = {3,3,3,3,3,3,3,3};
    const uint16_t addresses[8] = {0x8003, 0x8006, 0x8007, 0x8008, 0x8009, 0x800a, 0x800b, 0x800c};
    struct message message;
    tekon_req_1c(&message, 2, devices, addresses, NULL, 8);
    tekon_req_pack(buffer, sizeof(buffer), &message, 1);
}

MU_TEST(test_msg_readem_list_1c_inv_long)
{

    const uint8_t devices[8] = {3,3,3,3,3,3,3,3};
    const uint16_t addresses[8] = {0x8003, 0x8006, 0x8007, 0x8008, 0x8009, 0x800a, 0x800b, 0x800c};

    struct message message;
    int result = tekon_req_1c(&message, 2, devices, addresses, NULL, TEKON_PROTO_PLIST_SIZE + 1);
    mu_assert_int_eq(0,result);
}

MU_TEST(test_msg_readem_list_1c_inv_dev)
{

    const uint8_t devices[8] = {3,3,3,TEKON_INVALID_DEV_ADDR,3,3,3,3};
    const uint16_t addresses[8] = {0x8003, 0x8006, 0x8007, 0x8008, 0x8009, 0x800a, 0x800b, 0x800c};

    struct message message;
    int result = tekon_req_1c(&message, 2, devices, addresses, NULL, 8);
    mu_assert_int_eq(0,result);
}


MU_TEST(test_msg_readem_list_1c)
{

    const uint8_t devices[8] = {3,3,3,3,3,3,3,3};
    const uint16_t addresses[8] = {0x8003, 0x8006, 0x8007, 0x8008, 0x8009, 0x800a, 0x800b, 0x800c};

    struct message message;
    tekon_req_1c(&message, 2, devices, addresses, NULL, 8);
    mu_assert_int_eq(TEKON_MSG_READEM_PAR_LIST_1C, message.type);
    mu_assert_int_eq(TEKON_DIR_OUT, message.dir);
    mu_assert_int_eq(8, message.nelements);

    for (size_t i = 0; i < 8; i++) {
        mu_assert_int_eq(devices[i], message.payload.parameters[i].device);
        mu_assert_int_eq(addresses[i], message.payload.parameters[i].address);
    }
}

MU_TEST(test_pack_readem_list_1c)
{

    uint8_t buffer[1024] = {0};
    const uint8_t control_msg[] = {
        0x68, 0x33, 0x33, 0x68, 0x4b, 0x02, 0x1c, 0x03, 0x03, 0x80, 0x00, 0x00,
        0x00, 0x03, 0x06, 0x80, 0x00, 0x00, 0x00, 0x03, 0x07, 0x80, 0x00, 0x00,
        0x00, 0x03, 0x08, 0x80, 0x00, 0x00, 0x00, 0x03, 0x09, 0x80, 0x00, 0x00,
        0x00, 03,   0x0a, 0x80, 0x00, 0x00, 0x00, 0x03, 0x0b, 0x80, 0x00, 0x00,
        0x00, 0x03, 0x0c, 0x80, 0x00, 0x00, 0x00, 0xc3, 0x16
    };
    const uint8_t devices[8] = {3,3,3,3,3,3,3,3};
    const uint16_t addresses[8] = {0x8003, 0x8006, 0x8007, 0x8008, 0x8009, 0x800a, 0x800b, 0x800c};

    struct message message;
    tekon_req_1c(&message, 2, devices, addresses, NULL, 8);
    int result = tekon_req_pack(buffer, sizeof(buffer), &message, 0xb);

    mu_assert_int_eq(sizeof(control_msg), result);

    for (size_t i = 0; i < sizeof(control_msg); i++) {
        mu_assert_int_eq(buffer[i], control_msg[i]);
    }
}

MU_TEST_SUITE(suite_pack_common)
{
    MU_RUN_TEST(test_pack_nums);
}

MU_TEST_SUITE(suite_readem_11)
{
    MU_RUN_TEST(test_pack_readem_11);
}

MU_TEST_SUITE(suite_readem_11_inv)
{
    MU_RUN_TEST(test_pack_readem_11_inv_buffer);
}

MU_TEST_SUITE(suite_readem_14)
{
    MU_RUN_TEST(test_pack_writem_14_passwd);
    MU_RUN_TEST(test_pack_writem_14_write_reg);
}


MU_TEST_SUITE(suite_readem_19)
{
    MU_RUN_TEST(test_pack_readem_19);
}


MU_TEST_SUITE(suite_readem_list_1c)
{
    MU_RUN_TEST(test_msg_readem_list_1c);
    MU_RUN_TEST(test_pack_readem_list_1c);
}

MU_TEST_SUITE(suite_readem_list_1c_inv)
{
    MU_RUN_TEST(test_pack_readem_list_1c_inv_buffer);
    MU_RUN_TEST(test_msg_readem_list_1c_inv_long);
    MU_RUN_TEST(test_msg_readem_list_1c_inv_dev);
}

int main()
{
    MU_RUN_SUITE(suite_pack_common);
    MU_RUN_SUITE(suite_readem_11);
    MU_RUN_SUITE(suite_readem_11_inv);
    MU_RUN_SUITE(suite_readem_14);
    MU_RUN_SUITE(suite_readem_19);
    MU_RUN_SUITE(suite_readem_list_1c);
    MU_RUN_SUITE(suite_readem_list_1c_inv);
    MU_REPORT();
    return mu_get_fails();
}

#ifdef __cplusplus
}
#endif
