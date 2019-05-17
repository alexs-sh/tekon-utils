/* Copyright (c) 2019
 * Alexander Shirokov
 * Schneider Electric
 * See LICENSE for details. */

#ifdef __cplusplus
extern "C" {
#endif

#include "tekon/message.h"
#include <assert.h>
#include <string.h>

int tekon_req_11(struct message * self, uint8_t gateway, uint8_t device, uint16_t address)
{
    assert(self);

    if(gateway == TEKON_INVALID_DEV_ADDR ||
            device == TEKON_INVALID_DEV_ADDR)
        return 0;

    memset(self, 0, sizeof(*self));

    self->gateway = gateway;
    self->dir = TEKON_DIR_OUT;
    self->type = TEKON_MSG_READEM_PAR_11;
    self->nelements = 1;
    self->payload.parameters[0].device = device;
    self->payload.parameters[0].address = address;
    self->payload.parameters[0].qual = 0;
    self->payload.parameters[0].value = 0;
    return 1;
}

int tekon_resp_11(struct message * self, uint8_t gateway, uint32_t value)
{
    assert(self);

    if(gateway == TEKON_INVALID_DEV_ADDR)
        return 0;

    memset(self, 0, sizeof(*self));
    struct tekon_parameter * param = self->payload.parameters;
    self->gateway = gateway;
    self->dir = TEKON_DIR_IN;
    self->type = TEKON_MSG_READEM_PAR_11;
    self->nelements = 1;
    memcpy(&param->value, &value, sizeof(value));

    return 1;
}

int tekon_req_14(struct message * self, uint8_t gateway, const void * data, size_t size)
{
    assert(self);

    if(gateway == TEKON_INVALID_DEV_ADDR)
        return 0;

    memset(self, 0, sizeof(*self));

    self->gateway = gateway;
    self->dir = TEKON_DIR_OUT;
    self->type =TEKON_MSG_WRITEM_PAR_14;
    self->nelements = size;
    memcpy(&self->payload, data, size);
    return 1;

}

int tekon_resp_ack(struct message * self, int positive)
{
    assert(self);
    memset(self, 0, sizeof(*self));

    self->dir = TEKON_DIR_IN;
    self->type = positive ? TEKON_MSG_POS_ACK : TEKON_MSG_NEG_ACK;
    return 1;
}

int tekon_req_19(struct message * self, uint8_t gateway, uint8_t device, uint16_t address, uint16_t index, size_t size)
{
    assert(self);
    size_t i;
    if(gateway == TEKON_INVALID_DEV_ADDR || size > TEKON_PROTO_ILIST_SIZE)
        return 0;

    memset(self, 0, sizeof(*self));
    self->gateway = gateway;
    self->dir = TEKON_DIR_OUT;
    self->type = TEKON_MSG_READEM_IND_LIST_19;
    self->nelements = size;
    struct tekon_parameter * param = self->payload.parameters;
    for(i = 0; i < size; i++, param++) {
        param->device = device;
        param->address = address;
        param->index = index + i;
    }
    return 1;
}

int tekon_resp_19(struct message * self, uint8_t gateway, const uint32_t *values, size_t size)
{
    assert(self);
    if(gateway == TEKON_INVALID_DEV_ADDR || size > TEKON_PROTO_ILIST_SIZE)
        return 0;

    memset(self, 0, sizeof(*self));

    self->type = TEKON_MSG_READEM_IND_LIST_19;
    self->dir = TEKON_DIR_IN;
    self->gateway = gateway;
    self->nelements = size;
    struct tekon_parameter * param = self->payload.parameters;
    size_t i;
    for(i = 0; i < size; i++, param++) {
        param->value = *values++;
        param->qual = 0;
    }
    return 1;

}

int tekon_req_1c(struct message * self, uint8_t gateway, const uint8_t *devices, const uint16_t *addresses, const uint16_t *indexes, size_t size)
{
    assert(self);
    size_t i;
    if(gateway == TEKON_INVALID_DEV_ADDR || size > TEKON_PROTO_PLIST_SIZE)
        return 0;

    for(i = 0; i < size; i++) {
        if(devices[i] == TEKON_INVALID_DEV_ADDR)
            return 0;
    }

    memset(self, 0, sizeof(*self));
    self->gateway = gateway;
    self->dir = TEKON_DIR_OUT;
    self->type = TEKON_MSG_READEM_PAR_LIST_1C;
    self->nelements = size;
    struct tekon_parameter * param = self->payload.parameters;
    for(i = 0; i < size; i++, param++) {
        param->device = *devices++;
        param->address = *addresses++;

        if(indexes)
            param->index = *indexes++;
        else
            param->index = TEKON_INVALID_INDEX;
    }
    return 1;
}

int tekon_resp_1c(struct message * self, uint8_t gateway, const uint32_t *values, const uint8_t *quals, size_t size)
{
    assert(self);
    if(gateway == TEKON_INVALID_DEV_ADDR || size > TEKON_PROTO_PLIST_SIZE)
        return 0;

    memset(self, 0, sizeof(*self));

    self->type = TEKON_MSG_READEM_PAR_LIST_1C;
    self->dir = TEKON_DIR_IN;
    self->gateway = gateway;
    self->nelements = size;
    struct tekon_parameter * param = self->payload.parameters;
    size_t i;
    for(i = 0; i < size; i++, param++) {
        param->value = *values++;
        param->qual = *quals++;
    }
    return 1;
}


#ifdef __cplusplus
}
#endif


