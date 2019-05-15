/* Copyright (c) 2019
 * Alexander Shirokov
 * Schneider Electric
 * See LICENSE for details. */

#ifdef __cplusplus
extern "C" {
#endif

#include "test/minunit.h"
#include "tekon/unpack.h"

MU_TEST(test_read_pack)
{
    uint8_t ack = 0xA2;
    uint8_t num = 0;
    struct message msg;
    int result = tekon_resp_unpack(&ack, sizeof(ack), &msg, TEKON_MSG_READEM_PAR_11, &num);
    mu_assert_int_eq(1, result);
    mu_assert_int_eq(TEKON_MSG_POS_ACK, msg.type);
    mu_assert_int_eq(0, num);
}

MU_TEST(test_read_pack_en)
{
    uint8_t ack[] = {0xA2, 1, 2, 3};
    uint8_t num = 0;
    struct message msg;
    int result = tekon_resp_unpack(ack, sizeof(ack), &msg, TEKON_MSG_READEM_PAR_11, &num);
    mu_assert_int_eq(1, result);
    mu_assert_int_eq(TEKON_DIR_IN, msg.dir);
    mu_assert_int_eq(TEKON_MSG_POS_ACK, msg.type);
    mu_assert_int_eq(0, num);
}

MU_TEST(test_read_nack)
{
    uint8_t ack = 0xE5;
    uint8_t num = 0;
    struct message msg;
    int result = tekon_resp_unpack(&ack, sizeof(ack), &msg, TEKON_MSG_READEM_PAR_11, &num);
    mu_assert_int_eq(1, result);
    mu_assert_int_eq(TEKON_MSG_NEG_ACK, msg.type);
    mu_assert_int_eq(0, num);

}

MU_TEST(test_read_nack_en)
{
    uint8_t ack[] = {0xE5, 1, 2, 3};
    uint8_t num = 0;
    struct message msg;
    int result = tekon_resp_unpack(ack, sizeof(ack), &msg, TEKON_MSG_READEM_PAR_11, &num);
    mu_assert_int_eq(1, result);
    mu_assert_int_eq(TEKON_DIR_IN, msg.dir);
    mu_assert_int_eq(TEKON_MSG_NEG_ACK, msg.type);
    mu_assert_int_eq(0, num);

}

MU_TEST(test_read_readem_11)
{
    const uint8_t control_msg[] = {0x68, 0x06, 0x06, 0x68, 0x07, 0x02,
                                   0x00, 0x00, 0x16, 0x43, 0x62, 0x16
                                  };
    uint8_t control_data[] = {0x00, 0x00, 0x16, 0x43};
    uint32_t control_value;
    uint8_t num = 0;
    struct message msg;
    memcpy(&control_value, control_data, 4);
    int result = tekon_resp_unpack(control_msg, sizeof(control_msg), &msg, TEKON_MSG_READEM_PAR_11, &num);
    mu_assert_int_eq(12, result);
    mu_assert_int_eq(TEKON_DIR_IN, msg.dir);
    mu_assert_int_eq(TEKON_MSG_READEM_PAR_11, msg.type);
    mu_assert_int_eq(0x7, num);
    mu_assert_int_eq(0x2, msg.gateway);
    mu_assert_int_eq(1, msg.nelements);
    mu_assert_int_eq(control_value, msg.payload.parameters[0].value);

}

MU_TEST(test_read_readem_11_en)
{
    const uint8_t control_msg[] = {0x68, 0x06, 0x06, 0x68, 0x07, 0x02,
                                   0x00, 0x00, 0x16, 0x43, 0x62, 0x16, 11,22,33
                                  };
    uint8_t control_data[] = {0x00, 0x00, 0x16, 0x43};
    uint32_t control_value;
    uint8_t num = 0;
    struct message msg;
    memcpy(&control_value, control_data, 4);
    int result = tekon_resp_unpack(control_msg, sizeof(control_msg), &msg, TEKON_MSG_READEM_PAR_11, &num);
    mu_assert_int_eq(12, result);
    mu_assert_int_eq(TEKON_DIR_IN, msg.dir);
    mu_assert_int_eq(TEKON_MSG_READEM_PAR_11, msg.type);
    mu_assert_int_eq(0x7, num);
    mu_assert_int_eq(0x2, msg.gateway);
    mu_assert_int_eq(1, msg.nelements);
    mu_assert_int_eq(control_value, msg.payload.parameters[0].value);
}

MU_TEST(test_read_readem_11_no_num)
{
    const uint8_t control_msg[] = {0x68, 0x06, 0x06, 0x68, 0x07, 0x02,
                                   0x00, 0x00, 0x16, 0x43, 0x62, 0x16, 11,22,33
                                  };
    uint8_t control_data[] = {0x00, 0x00, 0x16, 0x43};
    uint32_t control_value;
    struct message msg ;
    memcpy(&control_value, control_data, 4);
    int result = tekon_resp_unpack(control_msg, sizeof(control_msg), &msg, TEKON_MSG_READEM_PAR_11, NULL);
    mu_assert_int_eq(12, result);
    mu_assert_int_eq(TEKON_DIR_IN, msg.dir);
    mu_assert_int_eq(TEKON_MSG_READEM_PAR_11, msg.type);
    mu_assert_int_eq(0x2, msg.gateway);
    mu_assert_int_eq(1, msg.nelements);
    mu_assert_int_eq(control_value, msg.payload.parameters[0].value);
}

MU_TEST(test_read_readem_11_inv_prefix)
{
    const uint8_t control_msg[] = {0x10, 0x06, 0x06, 0x68, 0x07, 0x02,
                                   0x00, 0x00, 0x16, 0x43, 0x62, 0x16
                                  };
    struct message msg ;
    int result = tekon_resp_unpack(control_msg, sizeof(control_msg), &msg, TEKON_MSG_READEM_PAR_11, 0);
    mu_assert_int_eq(0, result);
}

MU_TEST(test_read_readem_11_inv_crc)
{
    const uint8_t control_msg[] = {0x68, 0x06, 0x06, 0x68, 0x07, 0x02,
                                   0x00, 0x00, 0x16, 0x43, 0x52, 0x16
                                  };
    struct message msg ;
    int result = tekon_resp_unpack(control_msg, sizeof(control_msg), &msg, TEKON_MSG_READEM_PAR_11, 0);
    mu_assert_int_eq(0, result);
}

MU_TEST(test_read_readem_11_inv_sm)
{
    const uint8_t control_msg[] = {0x68, 0x06, 0x06, 0x68, 0x07, 0x02,
                                   0x00, 0x00, 0x16, 0x43, 0x62
                                  };
    uint8_t control_data[] = {0x00, 0x00, 0x16, 0x43};
    uint32_t control_value;
    uint8_t num = 0;
    struct message msg ;
    memcpy(&control_value, control_data, 4);
    int result = tekon_resp_unpack(control_msg, sizeof(control_msg), &msg, TEKON_MSG_READEM_PAR_11, &num);
    mu_assert_int_eq(0, result);
}

MU_TEST(test_read_readem_11_inv_len1)
{
    const uint8_t control_msg[] = {0x68, 0x02, 0x06, 0x68, 0x07, 0x02,
                                   0x00, 0x00, 0x16, 0x43, 0x52, 0x16
                                  };
    struct message msg ;
    int result = tekon_resp_unpack(control_msg, sizeof(control_msg), &msg, TEKON_MSG_READEM_PAR_11, 0);
    mu_assert_int_eq(0, result);
}

MU_TEST(test_read_readem_11_inv_len2)
{
    const uint8_t control_msg[] = {0x68, 0x08, 0x08, 0x68, 0x07, 0x02,
                                   0x00, 0x00, 0x16, 0x43, 0x52, 0x16
                                  };
    struct message msg ;
    int result = tekon_resp_unpack(control_msg, sizeof(control_msg), &msg, TEKON_MSG_READEM_PAR_11, 0);
    mu_assert_int_eq(0, result);
}

MU_TEST(test_read_readem_11_inv_len3)
{
    const uint8_t control_msg[] = {0x68, 0x02, 0x02, 0x68, 0x07, 0x02,
                                   0x00, 0x00, 0x16, 0x43, 0x52, 0x16
                                  };
    struct message msg ;
    int result = tekon_resp_unpack(control_msg, sizeof(control_msg), &msg, TEKON_MSG_READEM_PAR_11, 0);
    mu_assert_int_eq(0, result);
}

MU_TEST(test_read_writem_14_passwd)
{

    uint8_t control_msg[] = {
        0x68,0x03,0x03,0x68,0x02,0x09,0x02,0x0d,0x16
    };

    struct message message;
    uint8_t num;
    int result = tekon_resp_unpack(control_msg, sizeof(control_msg), &message, TEKON_MSG_WRITEM_PAR_14, &num);
    mu_assert_int_eq(sizeof(control_msg), result);
    mu_assert_int_eq(TEKON_MSG_WRITEM_PAR_14, message.type);
    mu_assert_int_eq(TEKON_DIR_IN, message.dir);
    mu_assert_int_eq(9,message.gateway);
    mu_assert_int_eq(2,num);
    mu_assert_int_eq(1,message.nelements);
    mu_assert_int_eq(2, message.payload.bytes[0]);
}

MU_TEST(test_read_writem_14_posack)
{

    uint8_t control_msg[] = {
        TEKON_PROTO_POS_ACK
    };
    struct message message;
    uint8_t num;
    int result = tekon_resp_unpack(control_msg, sizeof(control_msg), &message, TEKON_MSG_WRITEM_PAR_14, &num);
    mu_assert_int_eq(1, result);
    mu_assert_int_eq(TEKON_MSG_POS_ACK,message.type);
}

MU_TEST(test_read_writem_14_nack)
{

    uint8_t control_msg[] = {
        TEKON_PROTO_NEG_ACK
    };
    struct message message;
    uint8_t num;
    int result = tekon_resp_unpack(control_msg, sizeof(control_msg), &message, TEKON_MSG_WRITEM_PAR_14, &num);
    mu_assert_int_eq(1, result);
    mu_assert_int_eq(TEKON_MSG_NEG_ACK,message.type);
}

MU_TEST(test_read_readem_19)
{

    uint8_t control_msg[] = {0x68, 0x0e, 0x0e, 0x68, 0x06, 0x02, 0xd0,
                             0x61, 0x29, 0x46, 0xba, 0x0d, 0x31, 0x46,
                             0x3b, 0x65, 0x34, 0x46, 0x00, 0x16
                            };

    struct message message;
    uint8_t num;
    int result = tekon_resp_unpack(control_msg, sizeof(control_msg), &message, TEKON_MSG_READEM_IND_LIST_19, &num);
    mu_assert_int_eq(sizeof(control_msg), result);
    mu_assert_int_eq(2,message.gateway);
    mu_assert_int_eq(6,num);
    mu_assert_int_eq(3,message.nelements);

    float value1, value2, value3;

    memcpy(&value1, &message.payload.parameters[0].value, 4);
    memcpy(&value2, &message.payload.parameters[1].value, 4);
    memcpy(&value3, &message.payload.parameters[2].value, 4);

    // Чтобы тесты не срезались из за несовпадения точности в десятх, сотых,
    // тысячный, просто срваним их с округлением до целого. Это позволит и
    // нормально проверить и не лечить ненужную параною
    mu_assert_int_eq((int)(10840.453), value1);
    mu_assert_int_eq((int)11331.432, value2);
    mu_assert_int_eq((int)11545.308, value3);

}

MU_TEST(test_read_readem_1C)
{

    const uint8_t control_msg[] = {0x68, 0x2a, 0x2a, 0x68, 0x0b, 0x02,
                                   0x08, 0x7c, 0xde, 0x42, 0x00,  // <-- 1
                                   0x00, 0x00, 0x00, 0x00, 0x00,  // <-- 2
                                   0x00, 0x00, 0x16, 0x43, 0x00,  // <-- 3
                                   0x00, 0x00, 0x00, 0x00, 0x00,  // <-- 4
                                   0x00, 0x00, 0x16, 0x43, 0x00,  // <-- 5
                                   0x08, 0x7c, 0xde, 0x42, 0x00,  // <-- 6
                                   0x00, 0x00, 0x00, 0x00, 0x00,  // <-- 7
                                   0x00, 0x00, 0x00, 0x00, 0x00,  // <-- 8
                                   0x07, 0x16
                                  };
    uint8_t num = 0;
    struct message msg;
    int result = tekon_resp_unpack(control_msg, sizeof(control_msg), &msg, TEKON_MSG_READEM_PAR_LIST_1C, &num);
    mu_assert_int_eq(sizeof(control_msg), result);
    mu_assert_int_eq(TEKON_DIR_IN, msg.dir);
    mu_assert_int_eq(TEKON_MSG_READEM_PAR_LIST_1C, msg.type);
    mu_assert_int_eq(0x0b, num);
    mu_assert_int_eq(0x2, msg.gateway);
    mu_assert_int_eq(8, msg.nelements);

}

MU_TEST(test_read_readem_1C_inv_len)
{

    const uint8_t control_msg[] = {0x68, 0x2b, 0x2b, 0x68, 0x0b, 0x02,
                                   0x08, 0x7c, 0xde, 0x42, 0x00,  // <-- 1
                                   0x00, 0x00, 0x00, 0x00, 0x00,  // <-- 2
                                   0x00, 0x00, 0x16, 0x43, 0x00,  // <-- 3
                                   0x00, 0x00, 0x00, 0x00, 0x00,  // <-- 4
                                   0x00, 0x00, 0x16, 0x43, 0x00,  // <-- 5
                                   0x08, 0x7c, 0xde, 0x42, 0x00,  // <-- 6
                                   0x00, 0x00, 0x00, 0x00, 0x00,  // <-- 7
                                   0x00, 0x00, 0x00, 0x00, 0x00,  // <-- 8
                                   0x07, 0x16
                                  };
    uint8_t num = 0;
    struct message msg;
    int result = tekon_resp_unpack(control_msg, sizeof(control_msg), &msg, TEKON_MSG_READEM_PAR_LIST_1C, &num);
    mu_assert_int_eq(0, result);
}

MU_TEST(test_read_readem_1C_inv_addr)
{

    const uint8_t control_msg[] = {0x68, 0x2a, 0x2a, 0x68, 0x0b, 0x00,
                                   0x08, 0x7c, 0xde, 0x42, 0x00,  // <-- 1
                                   0x00, 0x00, 0x00, 0x00, 0x00,  // <-- 2
                                   0x00, 0x00, 0x16, 0x43, 0x00,  // <-- 3
                                   0x00, 0x00, 0x00, 0x00, 0x00,  // <-- 4
                                   0x00, 0x00, 0x16, 0x43, 0x00,  // <-- 5
                                   0x08, 0x7c, 0xde, 0x42, 0x00,  // <-- 6
                                   0x00, 0x00, 0x00, 0x00, 0x00,  // <-- 7
                                   0x00, 0x00, 0x00, 0x00, 0x00,  // <-- 8
                                   0x07, 0x16
                                  };
    uint8_t num = 0;
    struct message msg;
    int result = tekon_resp_unpack(control_msg, sizeof(control_msg), &msg, TEKON_MSG_READEM_PAR_LIST_1C, &num);
    mu_assert_int_eq(0, result);
}

MU_TEST_SUITE(suite_message_pos_ack)
{
    MU_RUN_TEST(test_read_pack);
    MU_RUN_TEST(test_read_pack_en);
}

MU_TEST_SUITE(suite_message_neg_ack)
{
    MU_RUN_TEST(test_read_nack);
    MU_RUN_TEST(test_read_nack_en);
}

MU_TEST_SUITE(suite_message_readem_11)
{
    MU_RUN_TEST(test_read_readem_11);
    MU_RUN_TEST(test_read_readem_11_en);
    MU_RUN_TEST(test_read_readem_11_no_num);
}

MU_TEST_SUITE(suite_message_readem_11_inv)
{
    MU_RUN_TEST(test_read_readem_11_inv_prefix);
    MU_RUN_TEST(test_read_readem_11_inv_sm);
    MU_RUN_TEST(test_read_readem_11_inv_crc);
    MU_RUN_TEST(test_read_readem_11_inv_len1);
    MU_RUN_TEST(test_read_readem_11_inv_len2);
    MU_RUN_TEST(test_read_readem_11_inv_len3);
}

MU_TEST_SUITE(suite_message_writem_14)
{
    MU_RUN_TEST(test_read_writem_14_passwd);
    MU_RUN_TEST(test_read_writem_14_posack);
    MU_RUN_TEST(test_read_writem_14_nack);
}

MU_TEST_SUITE(suite_message_readem_19)
{
    MU_RUN_TEST(test_read_readem_19);
}

MU_TEST_SUITE(suite_message_readem_1C)
{
    MU_RUN_TEST(test_read_readem_1C);
}

MU_TEST_SUITE(suite_message_readem_1C_inv)
{
    MU_RUN_TEST(test_read_readem_1C_inv_len);
    MU_RUN_TEST(test_read_readem_1C_inv_addr);
}

int main()
{
    MU_RUN_SUITE(suite_message_pos_ack);
    MU_RUN_SUITE(suite_message_neg_ack);
    MU_RUN_SUITE(suite_message_readem_11);
    MU_RUN_SUITE(suite_message_readem_11_inv);
    MU_RUN_SUITE(suite_message_writem_14);
    MU_RUN_SUITE(suite_message_readem_19);
    MU_RUN_SUITE(suite_message_readem_1C);
    MU_RUN_SUITE(suite_message_readem_1C_inv);
    MU_REPORT();
    return mu_get_fails();
}


#ifdef __cplusplus
}
#endif
