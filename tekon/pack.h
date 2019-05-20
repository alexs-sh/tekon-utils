/* Copyright (c) 2019
 * Alexander Shirokov
 * Schneider Electric
 * See LICENSE for details. */

#ifndef TEKON_PACK_H
#define TEKON_PACK_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>
#include "tekon/message.h"

/* Записть сообщение в буфер
 * В случае успеха возврщает кол-во записанных байт
 * 0 - ошибка */
ssize_t tekon_req_pack(void * buffer, size_t size, const struct message * message, uint8_t number);

#ifdef __cplusplus
}
#endif

#endif
