/* Copyright (c) 2019
 * Alexander Shirokov
 * Schneider Electric
 * See LICENSE for details. */

#ifndef TEKON_UNPACK_H
#define TEKON_UNPACK_H

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/types.h>
#include "tekon/message.h"

/* Записть сообщение в буфер
 * В случае успеха возврщает кол-во прочитанных байт
 * 0 - ошибка */
ssize_t tekon_resp_unpack(const void * buffer, size_t size, struct message * message, enum tekon_message_type type, uint8_t * number);


#ifdef __cplusplus
}
#endif

#endif
