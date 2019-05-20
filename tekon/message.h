/* Copyright (c) 2019
 * Alexander Shirokov
 * Schneider Electric
 * See LICENSE for details. */

#ifndef TEKON_MESSAGE_H
#define TEKON_MESSAGE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "tekon/proto.h"

struct message {

    /* Адрес шлюза */
    uint8_t gateway;

    /* Напрвяление (к / от устройства) */
    enum tekon_direction dir;

    /* Тип сообщения */
    enum tekon_message_type type;

    /* Кол-во элементов. Если сообщение предпологает передачу параметров,
     * то кол-во параметров в поле payload.parameters.
     * Если передачу байт, то кол-во байт в payload.bytes */
    uint8_t nelements;

    /* Данные */
    union {
        struct tekon_parameter parameters[TEKON_PROTO_ILIST_SIZE];
        uint8_t bytes[TEKON_PROTO_MAX_ADU_SIZE];
    } payload;

};

/* Сообщение "Чтение параметра из внешнего модуля 0x11"
 * T10.96.59РД-Д1 стр. 9
 * 1 - успешно
 * 0 - ошибка */
int tekon_req_11(struct message * self, uint8_t gateway, uint8_t device, uint16_t address);
int tekon_resp_11(struct message * self, uint8_t gateway, uint32_t value);

/* Сообщение "Передача во внешний модуль"
 * T10.96.59РД-Д1 стр. 9
 * 1 - успешно
 * 0 - ошибка */
int tekon_req_14(struct message * self, uint8_t gateway, const void * data, size_t size);

/* Сообщение "Чтение интдексного параметра внешнего модуля"
 * T10.96.59РД-Д1 стр. 16
 * 1 - успешно
 * 0 - ошибка */
int tekon_req_19(struct message * self, uint8_t gateway, uint8_t device, uint16_t address, uint16_t index, size_t size);
int tekon_resp_19(struct message * self, uint8_t gateway, const uint32_t *values, size_t size);

/* Сообщение "Чтение списка параметров из внешних модулуй 0x1C"
 * T10.96.59РД-Д1 стр. 23
 * 1 - успешно
 * 0 - ошибка */
int tekon_req_1c(struct message * self, uint8_t gateway, const uint8_t *devices, const uint16_t *addresses, const uint16_t *indexes, size_t size);
int tekon_resp_1c(struct message * self, uint8_t gateway, const uint32_t *values, const uint8_t *quals, size_t size);

/* Квитанция */
int tekon_resp_ack(struct message * self, int positive);

#ifdef __cplusplus
}
#endif

#endif
