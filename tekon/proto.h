/* Copyright (c) 2019
 * Alexander Shirokov
 * Schneider Electric
 * See LICENSE for details. */

#ifndef TEKON_PROTO_H
#define TEKON_PROTO_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

#define TEKON_INVALID_DEV_ADDR    0
#define TEKON_INVALID_INDEX       0xFFFF /* Признак невалидного индекса при четнии индексированных параметров */

#define TEKON_PASSWD_LEN      8
#define TEKON_PRIV_ADMIN      2
#define TEKON_PRIV_USER       1

#define TEKON_PROTO_MAX_ADU_SIZE    255
#define TEKON_PROTO_PLIST_SIZE      40  /* Длина списка параметров */
#define TEKON_PROTO_ILIST_SIZE      60  /* Длина списка инд. параметров */
#define TEKON_PROTO_POS_ACK         0xA2
#define TEKON_PROTO_NEG_ACK         0xE5
#define TEKON_PROTO_FIX_PREFIX      0x10
#define TEKON_PROTO_VAR_PREFIX      0x68
#define TEKON_PROTO_END             0x16


enum tekon_direction      { TEKON_DIR_IN,
                            TEKON_DIR_OUT,
                          };

enum tekon_parameter_type { TEKON_PARAM_RAW,
                            TEKON_PARAM_BOOL,
                            TEKON_PARAM_U32,
                            TEKON_PARAM_F32,
                            TEKON_PARAM_TIME,
                            TEKON_PARAM_DATE,
                            TEKON_PARAM_HEX
                          };

enum tekon_message_type { TEKON_MSG_UNK,
                          TEKON_MSG_POS_ACK,
                          TEKON_MSG_NEG_ACK,
                          TEKON_MSG_READEM_PAR_11,
                          TEKON_MSG_WRITEM_PAR_14,
                          TEKON_MSG_READEM_IND_LIST_19,
                          TEKON_MSG_READEM_PAR_LIST_1C,
                        };

struct tekon_parameter {
    uint8_t device;
    uint16_t address;
    uint16_t index;
    uint32_t value;
    uint8_t qual; /* значения байт качества (0 - ОК) */
};

uint8_t tekon_crc(const void * buffer, size_t size);
uint8_t tekon_fixed_crc(const void * buffer, size_t size);
uint8_t tekon_variable_crc(const void * buffer, size_t size);


#ifdef __cplusplus
}
#endif

#endif
