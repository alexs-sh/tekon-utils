/* Copyright (c) 2019
 * Alexander Shirokov
 * Schneider Electric
 * See LICENSE for details. */

#ifdef __cplusplus
extern "C" {
#endif

#include "tekon/unpack.h"
#include <assert.h>
#include <string.h>

struct buffer_reader {
    size_t avail;
    char * pos;
};

static void buffer_reader_init(struct buffer_reader * self, const void * buffer, size_t size);

// Запись значений в буфер.
// 1 - успешно
// 0 - ошибка
static int buffer_reader_u8(struct buffer_reader * self, uint8_t * u8);
static int buffer_reader_u32(struct buffer_reader * self, uint32_t * u32);
//static int buffer_reader_u16(struct buffer_reader * self, uint16_t * u16);

static ssize_t unpack_readem_11(const void * buffer, size_t size, struct message * message);
static ssize_t unpack_readem_14(const void * buffer, size_t size, struct message * message);
static ssize_t unpack_readem_19(const void * buffer, size_t size, struct message * message);
static ssize_t unpack_readem_list_1C(const void * buffer, size_t size, struct message * message);
static int validate(const void * buffer, ssize_t ssize);

// Записть сообщение в буфер
// В случае успеха возврщает кол-во прочитанных байт
// 0 - ошибка
ssize_t tekon_resp_unpack(const void * buffer, size_t size, struct message * message, enum tekon_message_type type, uint8_t * number)
{
    assert(buffer);
    assert(size);
    assert(message);
    // Быстрые проверки сообщения
    // - сообщение не может быть меньше 8 байт (кроме квитанций)
    // - начинается с 0x10 или 0x68 (кроме квитанций)
    // - номер посылки имеет фискированное положение (2 байт для фикс. сообщ. и 5
    // для переменныз сообщ.)

    // Грубая проверка - длина + КС
    if(!validate(buffer,size))
        return 0;

    // Самый простой вариант - подтверждение
    const uint8_t * ptr = buffer;
    const uint8_t start = ptr[0];

    if(start == TEKON_PROTO_POS_ACK || start == TEKON_PROTO_NEG_ACK) {
        tekon_resp_ack(message, start == TEKON_PROTO_POS_ACK);
        return 1;
    }

    // извлечение номера сообщения
    if(number) {

        if(start == TEKON_PROTO_FIX_PREFIX)
            *number = ptr[1] & 0x0F;
        else if(start == TEKON_PROTO_VAR_PREFIX)
            *number = ptr[4] & 0x0F;
        // ACK / NACK без номера
    }

    // разбор сообщения
    switch(type) {
    case TEKON_MSG_READEM_PAR_11:
        return unpack_readem_11(buffer, size, message);
    case TEKON_MSG_READEM_IND_LIST_19:
        return unpack_readem_19(buffer, size, message);
    case TEKON_MSG_READEM_PAR_LIST_1C:
        return unpack_readem_list_1C(buffer, size, message);
    case TEKON_MSG_WRITEM_PAR_14:
        return unpack_readem_14(buffer, size, message);
    case TEKON_MSG_POS_ACK:
    case TEKON_MSG_NEG_ACK:
    case TEKON_MSG_UNK:
        break;
    }
    return 0;
}

static int validate(const void * buffer, ssize_t size)
{

    if(size <= 0)
        return 0;

    const uint8_t * ptr = buffer;
    uint8_t crc = 0;
    uint8_t msg_crc = 255;
    uint8_t msg_eom = 255;

    switch(*ptr) {
    case TEKON_PROTO_POS_ACK:
    case TEKON_PROTO_NEG_ACK:
        return 1;

    case TEKON_PROTO_FIX_PREFIX:
        if(size >= 9) {
            crc = tekon_fixed_crc(buffer,size);
            msg_crc = ptr[7];
            msg_eom = ptr[8];
        }
        break;
    case TEKON_PROTO_VAR_PREFIX:
        if(size >= 9 &&
                size >= ptr[1] + 6 &&
                ptr[1] == ptr[2] &&
                ptr[1] > 2) {

            const uint8_t len = ptr[1] + 6u;
            crc = tekon_variable_crc(buffer, len);
            msg_crc = ptr [len - 2];
            msg_eom = ptr [len - 1];
        }
        break;
    default:
        return 0;
    }

    return crc == msg_crc &&
           msg_eom == TEKON_PROTO_END;
}


static ssize_t unpack_readem_11(const void * buffer, size_t size, struct message * message)
{
    struct buffer_reader reader;
    buffer_reader_init(&reader, buffer, size);
    const uint8_t * ptr = buffer;
    uint8_t start1 = 0;
    uint8_t start2 = 0;
    uint8_t len1 = 0;
    uint8_t len2 = 0;
    uint8_t num = 0;
    uint8_t gateway = 0;

    uint32_t value = 0;

    buffer_reader_u8(&reader, &start1);
    buffer_reader_u8(&reader, &len1);
    buffer_reader_u8(&reader, &len2);
    buffer_reader_u8(&reader, &start2);
    buffer_reader_u8(&reader, &num);
    buffer_reader_u8(&reader, &gateway);

    if(start1 != start2 || start1 != TEKON_PROTO_VAR_PREFIX)
        return 0;

    if(len1 != len2 || len1 < 3)
        return 0;

    len1 -= 2;

    if (len1 > sizeof(value))
        return 0;

    const uint8_t *data = &ptr[6];
    memcpy(&value, data, len1);
    tekon_resp_11(message, gateway, value);
    return len1 + 8;
}

static ssize_t unpack_readem_14(const void * buffer, size_t size, struct message * message)
{
    const uint8_t * ptr = buffer;
    if(size == 1) {
        tekon_resp_ack(message, *ptr == TEKON_MSG_POS_ACK);
        return 1;
    } else {
        struct buffer_reader reader;
        buffer_reader_init(&reader, buffer, size);
        uint8_t start1 = 0;
        uint8_t start2 = 0;
        uint8_t len1 = 0;
        uint8_t len2 = 0;
        uint8_t num = 0;
        uint8_t gateway = 0;
        uint8_t level = 0;

        buffer_reader_u8(&reader, &start1);
        buffer_reader_u8(&reader, &len1);
        buffer_reader_u8(&reader, &len2);
        buffer_reader_u8(&reader, &start2);
        buffer_reader_u8(&reader, &num);
        buffer_reader_u8(&reader, &gateway);
        buffer_reader_u8(&reader, &level);

        if(start1 == TEKON_PROTO_VAR_PREFIX &&
                start1 == start2 &&
                len1 == 3 &&
                len1 == len2 &&
                gateway != TEKON_INVALID_DEV_ADDR) {
            message->gateway = gateway;
            message->nelements = 1;
            message->payload.bytes[0] = level;
            message->type = TEKON_MSG_WRITEM_PAR_14;
            message->dir = TEKON_DIR_IN;
            return 9;
        }

    }

    return 0;

}

static ssize_t unpack_readem_19(const void * buffer, size_t size, struct message * message)
{
    assert(buffer);
    assert(message);

    struct buffer_reader reader;
    buffer_reader_init(&reader, buffer, size);
    uint8_t start1 = 0;
    uint8_t start2 = 0;
    uint8_t len1 = 0;
    uint8_t len2 = 0;
    uint8_t num = 0;
    uint8_t gateway = 0;

    uint32_t values[TEKON_PROTO_ILIST_SIZE] = {0};

    buffer_reader_u8(&reader, &start1);
    buffer_reader_u8(&reader, &len1);
    buffer_reader_u8(&reader, &len2);
    buffer_reader_u8(&reader, &start2);
    buffer_reader_u8(&reader, &num);
    buffer_reader_u8(&reader, &gateway);

    if(start1 != start2 || start1 != TEKON_PROTO_VAR_PREFIX)
        return 0;

    if(len1 != len2 || len1 < 8)
        return 0;

    len1 -= 2;

    if (len1 % 4  != 0)
        return 0;

    size_t i;
    const uint8_t nelem = len1 / 4;
    for(i = 0; i < nelem; i++) {
        buffer_reader_u32(&reader, &values [i]);
    }

    tekon_resp_19(message, gateway, values, nelem);
    return len1 + 8;

}

static ssize_t unpack_readem_list_1C(const void * buffer, size_t size, struct message * message)
{
    assert(buffer);
    assert(message);
    struct buffer_reader reader;
    buffer_reader_init(&reader, buffer, size);
    uint8_t start1 = 0;
    uint8_t start2 = 0;
    uint8_t len1 = 0;
    uint8_t len2 = 0;
    uint8_t num = 0;
    uint8_t gateway = 0;

    uint8_t quals[TEKON_PROTO_PLIST_SIZE] = {0};
    uint32_t values[TEKON_PROTO_PLIST_SIZE] = {0};

    buffer_reader_u8(&reader, &start1);
    buffer_reader_u8(&reader, &len1);
    buffer_reader_u8(&reader, &len2);
    buffer_reader_u8(&reader, &start2);
    buffer_reader_u8(&reader, &num);
    buffer_reader_u8(&reader, &gateway);

    if(start1 != start2 || start1 != TEKON_PROTO_VAR_PREFIX)
        return 0;

    if(len1 != len2 || len1 < 7)
        return 0;

    len1 -= 2;

    if (len1 % 5  != 0)
        return 0;

    size_t i;
    const uint8_t nelem = len1 / 5;
    for(i = 0; i < nelem; i++) {
        buffer_reader_u32(&reader, &values [i]);
        buffer_reader_u8(&reader, &quals [i]);
    }

    tekon_resp_1c(message, gateway, values, quals, nelem);
    return len1 + 8;
}


static void buffer_reader_init(struct buffer_reader * self, const void * buffer, size_t size)
{
    assert(self);
    assert(buffer);
    assert(size);
    self->pos = (char*)buffer;
    self->avail = size;
}

// Запись значений в буфер.
// 1 - успешно
// 0 - ошибка
static int buffer_reader_u8(struct buffer_reader * self, uint8_t * u8)
{
    assert(self);
    assert(u8);

    const size_t size = sizeof(*u8);

    if(self->avail < size)
        return 0;

    memcpy(u8, self->pos, size);
    self->pos+=size;
    self->avail-=size;
    return 1;
}

/*static int buffer_reader_u16(struct buffer_reader * self, uint16_t * u16)
{

    assert(self);
    assert(u16);

    const size_t size = sizeof(*u16);

    if(self->avail < size)
        return 0;

    memcpy(u16, self->pos, size);
    self->pos+=size;
    self->avail-=size;
    return 1;

}*/

static int buffer_reader_u32(struct buffer_reader * self, uint32_t * u32)
{
    assert(self);
    assert(u32);

    const size_t size = sizeof(*u32);

    if(self->avail < size)
        return 0;

    memcpy(u32, self->pos, size);
    self->pos+=size;
    self->avail-=size;
    return 1;

}


#ifdef __cplusplus
}
#endif


