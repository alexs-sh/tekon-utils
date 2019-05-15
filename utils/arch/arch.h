/* Copyright (c) 2019
 * Alexander Shirokov
 * Schneider Electric
 * See LICENSE for details. */

#ifndef UTILS_ARCH_ARCH_H
#define UTILS_ARCH_ARCH_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "tekon/proto.h"
#include "tekon/time.h"
#include "utils/base/time.h"
#include "utils/base/tstamp.h"
#include "utils/base/types.h"

#define ARCHIVE_MAX_SIZE  TIMESTAMP_MAX_SEQ_SIZE

struct devtime {
    struct tekon_date date;
    struct tekon_time time;
};

struct rec {
    uint16_t index;
    enum quality qual;
    int64_t timestamp;
    union {
        float f32;
        uint32_t u32;
        uint8_t byte[4];
    } value;
};

void rec_init(struct rec * self, uint16_t index);

void rec_update(struct rec * self, enum quality qual, const void * data, size_t size);


struct archive {
    struct paraddr address;
    struct intcfg interval;
    size_t size;
    struct rec rec[ARCHIVE_MAX_SIZE];
};

void archive_init(struct archive * self);

struct rec * archive_get(struct archive * self, size_t index);

int archive_add(struct archive * self, const struct rec * rec);

size_t archive_size(const struct archive * self);

void archive_foreach(struct archive * self, void (*visitor)(struct rec * rec, void * data), void * data);

int archive_index_to_utc(struct archive * self, const struct devtime * from, const struct devtime * to);

#ifdef __cplusplus
}
#endif

#endif
