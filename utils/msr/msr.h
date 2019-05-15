/* Copyright (c) 2019
 * Alexander Shirokov
 * Schneider Electric
 * See LICENSE for details. */

#ifndef UTILS_MSR_MSR_H
#define UTILS_MSR_MSR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "tekon/proto.h"
#include "utils/base/types.h"

// Макс. кол-вол измерений, которое может быть запрошено
// за один сеанс.
#define MEASURMENT_MAX_TABLE_SIZE 1024

struct msr {
    uint8_t gateway;
    uint8_t device;
    uint16_t address;
    uint16_t index;
    enum tekon_parameter_type type;
    enum quality qual;
    int64_t timestamp;
    union {
        float f32;
        uint32_t u32;
        uint8_t byte[4];
    } value;
    char hex;
};

void msr_init(struct msr * self, uint8_t gateway, uint8_t device, uint16_t address, uint16_t index, enum tekon_parameter_type type, char hex);

void msr_update(struct msr * self, enum quality qual, int64_t timestamp, const void * data, size_t size);


struct msr_table {
    struct msr msr[MEASURMENT_MAX_TABLE_SIZE];
    size_t size;
};

void msr_table_init(struct msr_table * self);


struct msr * msr_table_get(struct msr_table * self, size_t index);


int msr_table_add(struct msr_table * self, const struct msr * msr);

size_t msr_table_size(const struct msr_table * self);

void msr_table_foreach(struct msr_table * self, void (*visitor)(struct msr * msr, void * data), void * data);

#ifdef __cplusplus
}
#endif

#endif
