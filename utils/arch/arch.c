/* Copyright (c) 2019
 * Alexander Shirokov
 * Schneider Electric
 * See LICENSE for details. */

#ifdef __cplusplus
extern "C" {
#endif

#include "utils/arch/arch.h"
#include <assert.h>
#include <string.h>
#include "utils/base/time.h"

/* сравнить индексы 2-х меток времени */
static int index_eq(const struct devtime * begin, const struct devtime * end, const struct intcfg * interval)
{
    assert(begin);
    assert(end);
    assert(interval);
    int bdx = -10, edx = -11;
    switch(interval->type) {
    case 'm':
        bdx = tekon_month_index(begin->date.year, begin->date.month, interval->depth);
        edx = tekon_month_index(end->date.year, end->date.month, interval->depth);
        break;
    case 'd':
        bdx = tekon_day_index(begin->date.year, begin->date.month, begin->date.day);
        edx = tekon_day_index(end->date.year, end->date.month, end->date.day);
        break;
    case 'h':
        bdx = tekon_hour_index(begin->date.year, begin->date.month, begin->date.day, begin->time.hour, interval->depth);
        edx = tekon_hour_index(end->date.year, end->date.month, end->date.day, end->time.hour, interval->depth);
        break;
    case 'i':
        bdx = tekon_interval_index(begin->date.year, begin->date.month, begin->date.day, begin->time.hour, begin->time.minute, interval->depth, interval->interval);
        edx = tekon_interval_index(end->date.year, end->date.month, end->date.day, end->time.hour, end->time.minute, interval->depth, interval->interval);
        break;
    }
    return bdx == edx;
}

/* Установить метку времени каждой архивной записи */
static void apply_time(struct rec * rec, void * data)
{
    struct timestamp_seq * seq = data;
    rec->timestamp = timestamp_seq_get(seq, rec->index);
}


void rec_init(struct rec * self, uint16_t index)
{
    assert(self);
    memset(self, 0, sizeof(*self));
    self->index = index;
    self->qual = Q_UNK;
    self->timestamp = TIME_INVALID;
}

void rec_update(struct rec * self, enum quality qual, const void * data, size_t size)
{
    assert(self);

    self->qual = qual;

    if(data && size) {
        assert(size <= 4);
        memcpy(&self->value, data, size);
    }
}


void archive_init(struct archive * self)
{
    assert(self);
    memset(self, 0, sizeof(*self));
}

struct rec * archive_get(struct archive * self, size_t index)
{
    assert(self);
    return index < ARCHIVE_MAX_SIZE ? &self->rec[index] : NULL;
}

int archive_add(struct archive * self, const struct rec * rec)
{
    assert(self);
    assert(rec);

    if(self->size >= ARCHIVE_MAX_SIZE)
        return 0;

    self->rec[self->size++] = *rec;
    return 1;
}

size_t archive_size(const struct archive * self)
{
    assert(self);
    return self->size;
}

void archive_foreach(struct archive * self, void (*visitor)(struct rec * rec, void * data), void * data)
{
    assert(self);
    assert(visitor);
    size_t i;
    for(i = 0; i < self->size; i++)
        visitor(self->rec + i, data);
}


int archive_index_to_utc(struct archive * self, const struct devtime * from, const struct devtime * to)
{
    assert(self);
    assert(from);
    assert(to);

    /* Нет реальных данных для конвертирования (конвертация заблокированна или
     * были ошибки по связи) */

    if(to->date.day == 0)
        return 0;

    /* Проверить, что за время чтения архива не случилось изменения индекса.
     * Иначе архив будет битым */
    if(!index_eq(from, to, &self->interval))
        return 0;

    struct timestamp_seq seq;
    timestamp_seq_init(&seq);
    switch(self->interval.type) {
    case 'm':
        timestamp_seq_month(&seq, &to->date, self->interval.depth);
        break;
    case 'd':
        timestamp_seq_day(&seq, &to->date);
        break;
    case 'h':
        timestamp_seq_hour(&seq, &to->date, &to->time, self->interval.depth);
        break;
    case 'i':
        timestamp_seq_interval(&seq, &to->date, &to->time, self->interval.depth, self->interval.interval);
        break;
    }
    archive_foreach(self, apply_time, &seq);
    return 1;
}

#ifdef __cplusplus
}
#endif




