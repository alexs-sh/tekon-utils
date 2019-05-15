/* Copyright (c) 2019
 * Alexander Shirokov
 * Schneider Electric
 * See LICENSE for details. */

#ifdef __cplusplus
extern "C" {
#endif

#include "utils/msr/msr.h"
#include <assert.h>
#include <string.h>
#include "utils/base/time.h"

void msr_init(struct msr * self, uint8_t gateway, uint8_t device, uint16_t address, uint16_t index, enum tekon_parameter_type type, char hex)
{
    assert(self);
    self->gateway = gateway;
    self->device = device;
    self->address = address;
    self->index = index;
    self->type = type;
    self->qual = Q_UNK;
    self->timestamp = TIME_INVALID;
    self->value.u32 = 0;
    self->hex = hex;
}

void msr_update(struct msr * self, enum quality qual, int64_t timestamp, const void * data, size_t size)
{
    assert(self);

    self->qual = qual;
    self->timestamp = timestamp;

    if(data && size) {
        assert(size <= 4);
        memcpy(&self->value, data, size);
    }
}


void msr_table_init(struct msr_table * self)
{
    assert(self);
    memset(self, 0, sizeof(*self));
}

struct msr * msr_table_get(struct msr_table * self, size_t index)
{
    assert(self);
    return index < MEASURMENT_MAX_TABLE_SIZE ? &self->msr[index] : NULL;
}

int msr_table_add(struct msr_table * self, const struct msr * msr)
{
    assert(self);
    assert(msr);

    if(self->size >= MEASURMENT_MAX_TABLE_SIZE)
        return 0;

    self->msr[self->size++] = *msr;
    return 1;
}

size_t msr_table_size(const struct msr_table * self)
{
    assert(self);
    return self->size;
}

void msr_table_foreach(struct msr_table * self, void (*visitor)(struct msr * msr, void * data), void *data)
{
    assert(self);
    assert(visitor);
    size_t i;
    for(i = 0; i < self->size; i++)
        visitor(self->msr + i, data);
}



#ifdef __cplusplus
}
#endif




