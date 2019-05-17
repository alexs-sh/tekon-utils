/* Copyright (c) 2019
 * Alexander Shirokov
 * Schneider Electric
 * See LICENSE for details. */

#ifdef __cplusplus
extern "C" {
#endif

#include "tekon/pack.h"
#include <assert.h>
#include <string.h>

struct buffer_writer {
    size_t avail;
    char * pos;
};

static void buffer_writer_init(struct buffer_writer * self, void * buffer, size_t size);

// Запись значений в буфер.
// 1 - успешно
// 0 - ошибка
static int buffer_writer_u8(struct buffer_writer * self, uint8_t u8);
static int buffer_writer_u16(struct buffer_writer * self, uint16_t u16);
//static int buffer_writer_u32(struct buffer_writer * self, uint32_t u32);


static ssize_t pack_readem_11(void * buffer, size_t size, const struct message * message, uint8_t number);
static ssize_t pack_readem_14(void * buffer, size_t size, const struct message * message, uint8_t number);
static ssize_t pack_readem_19(void * buffer, size_t size, const struct message * message, uint8_t number);
static ssize_t pack_readem_list_1C(void * buffer, size_t size, const struct message * message, uint8_t number);


ssize_t tekon_req_pack(void * buffer, size_t size, const struct message * message, uint8_t number)
{
    assert(message);
    assert(buffer);
    assert(size);

    const uint8_t max_number = 15;

    if(number > max_number)
        return 0;

    switch(message->type) {
    case TEKON_MSG_READEM_PAR_11:
        return pack_readem_11(buffer, size, message, number);
    case TEKON_MSG_READEM_IND_LIST_19:
        return pack_readem_19(buffer, size, message, number);
    case TEKON_MSG_READEM_PAR_LIST_1C:
        return pack_readem_list_1C(buffer, size, message, number);
    case TEKON_MSG_WRITEM_PAR_14:
        return pack_readem_14(buffer, size, message, number);
    case TEKON_MSG_POS_ACK:
    case TEKON_MSG_NEG_ACK:
    case TEKON_MSG_UNK:
        break;
    }

    return 0;
}

static ssize_t pack_readem_11(void * buffer, size_t size, const struct message * message, uint8_t number)
{
    // Лимиты для этого типа сообщений
    const uint8_t frame_size = 9;
    const uint8_t code = 0x11;

    if(size < frame_size)
        return 0;


    if(message->nelements != 1)
        return 0;

    struct buffer_writer writer;
    buffer_writer_init(&writer, buffer, size);
    buffer_writer_u8(&writer, TEKON_PROTO_FIX_PREFIX);
    buffer_writer_u8(&writer, 0x40 | number);
    buffer_writer_u8(&writer, message->gateway);
    buffer_writer_u8(&writer, code);

    const struct tekon_parameter * param = message->payload.parameters;
    buffer_writer_u8(&writer, param->device);
    buffer_writer_u16(&writer, param->address);

    const uint8_t crc = tekon_fixed_crc(buffer, frame_size);

    buffer_writer_u8(&writer, crc);
    buffer_writer_u8(&writer, TEKON_PROTO_END);
    return frame_size;
}

static ssize_t pack_readem_14(void * buffer, size_t size, const struct message * message, uint8_t number)
{
    assert(buffer);
    assert(message);
    assert(message->nelements > 3);

    //Т10.06.59РД-Д1 стр. 10-12
    const uint8_t nelem = message->nelements;
    const uint8_t code = 0x14;
    const uint8_t command = message->payload.bytes[2];


    struct buffer_writer writer;
    size_t frame_size =0;
    uint8_t len = 0;

    switch(command) {
    // Запись регистра
    case 0x03:

        frame_size = 18;
        len = 12;
        break;
    // Установка уровня доступа
    case 0x05:
        frame_size = 17;
        len =  11;
        break;
    default:
        assert(command == 0x03 ||
               command == 0x05);
    }
    if(frame_size == 0 ||
            size < frame_size)
        return 0;

    buffer_writer_init(&writer, buffer, size);
    buffer_writer_u8(&writer, TEKON_PROTO_VAR_PREFIX);
    buffer_writer_u8(&writer, len);
    buffer_writer_u8(&writer, len);
    buffer_writer_u8(&writer, TEKON_PROTO_VAR_PREFIX);
    buffer_writer_u8(&writer, 0x40 | number);
    buffer_writer_u8(&writer, message->gateway);
    buffer_writer_u8(&writer, code);

    size_t i;
    for(i = 0; i < nelem; i++)
        buffer_writer_u8(&writer, message->payload.bytes[i]);

    const uint8_t crc = tekon_variable_crc(buffer, frame_size);
    buffer_writer_u8(&writer, crc);
    buffer_writer_u8(&writer, TEKON_PROTO_END);
    return frame_size;

}

static ssize_t pack_readem_19(void * buffer, size_t size, const struct message * message, uint8_t number)
{
    assert(buffer);
    assert(message);

    const uint8_t nelem = message->nelements;
    const uint8_t frame_size = 15;
    const uint8_t len = 9;
    const uint8_t code = 0x19;
    const struct tekon_parameter * param = message->payload.parameters;

    if(size < frame_size)
        return 0;

    if(nelem == 0 ||
            nelem > TEKON_PROTO_ILIST_SIZE)
        return 0;

    struct buffer_writer writer;
    buffer_writer_init(&writer, buffer, size);
    buffer_writer_u8(&writer, TEKON_PROTO_VAR_PREFIX);
    buffer_writer_u8(&writer, len);
    buffer_writer_u8(&writer, len);
    buffer_writer_u8(&writer, TEKON_PROTO_VAR_PREFIX);
    buffer_writer_u8(&writer, 0x40 | number);
    buffer_writer_u8(&writer, message->gateway);
    buffer_writer_u8(&writer, code);
    buffer_writer_u8(&writer, param->device);
    buffer_writer_u16(&writer,param->address);
    buffer_writer_u16(&writer,param->index);
    buffer_writer_u8(&writer, nelem);

    const uint8_t crc = tekon_variable_crc(buffer, frame_size);

    buffer_writer_u8(&writer, crc);
    buffer_writer_u8(&writer, TEKON_PROTO_END);
    return frame_size;

}

static ssize_t pack_readem_list_1C(void * buffer, size_t size, const struct message * message, uint8_t number)
{
    // Лимиты для этого типа сообщений
    const uint8_t nelem = message->nelements;
    const uint8_t frame_size = nelem * 6 + 9;
    const uint8_t len = nelem * 6 + 3;
    const uint8_t code = 0x1C;

    if(size < frame_size)
        return 0;

    if(message->nelements > TEKON_PROTO_PLIST_SIZE)
        return 0;

    struct buffer_writer writer;
    buffer_writer_init(&writer, buffer, size);
    buffer_writer_u8(&writer, TEKON_PROTO_VAR_PREFIX);
    buffer_writer_u8(&writer, len);
    buffer_writer_u8(&writer, len);
    buffer_writer_u8(&writer, TEKON_PROTO_VAR_PREFIX);
    buffer_writer_u8(&writer, 0x40 | number);
    buffer_writer_u8(&writer, message->gateway);
    buffer_writer_u8(&writer, code);

    size_t i;
    const struct tekon_parameter * param = message->payload.parameters;
    for(i = 0; i < nelem; i++, param++) {

        buffer_writer_u8(&writer,param->device);
        buffer_writer_u16(&writer,param->address);

        if(param->index == TEKON_INVALID_INDEX) {
            buffer_writer_u16(&writer,0);
            buffer_writer_u8(&writer,0);
        } else {
            buffer_writer_u16(&writer,param->index);
            buffer_writer_u8(&writer,1);
        }
    }

    const uint8_t crc = tekon_variable_crc(buffer, frame_size);

    buffer_writer_u8(&writer, crc);
    buffer_writer_u8(&writer, TEKON_PROTO_END);
    return frame_size;

}

// Копирование в буфер везде сделано через memcpy.
// во-первых это устраняет проблемы с невыровненным доступом к памяти (ARMv5 например)
// во-вторых большинство компиляторов могут правильно понять и оптимизировать
// этот код исключив реальный вызов функции.
// Итого, memcpy обеспечивает переносимый и очень быстрый способ записи в буфер
static void buffer_writer_init(struct buffer_writer * self, void * buffer, size_t size)
{
    assert(self);
    assert(buffer);
    assert(size);
    self->avail = size;
    self->pos = buffer;
}

static int buffer_writer_u8(struct buffer_writer * self, uint8_t u8)
{
    assert(self);
    const size_t size = sizeof(u8);

    if(self->avail < size)
        return 0;

    memcpy(self->pos, &u8, size);
    self->pos+=size;
    self->avail-=size;
    return 1;
}

static int buffer_writer_u16(struct buffer_writer * self, uint16_t u16)
{
    assert(self);
    const size_t size = sizeof(u16);

    if(self->avail<size)
        return 0;

    memcpy(self->pos, &u16, size);
    self->pos+=size;
    self->avail-=size;
    return 1;
}


/*static int buffer_writer_u32(struct buffer_writer * self, uint32_t u32)
{
    assert(self);
    const size_t size = sizeof(u32);

    if(self->avail<size)
        return 0;

    memcpy(self->pos, &u32, size);
    self->pos+=size;
    self->avail-=size;
    return 1;
}*/


#ifdef __cplusplus
}
#endif


