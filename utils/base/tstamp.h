/* Copyright (c) 2019
 * Alexander Shirokov
 * Schneider Electric
 * See LICENSE for details. */

#ifndef UTILS_BASE_TSTAMP_H
#define UTILS_BASE_TSTAMP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include <time.h>
#include "tekon/time.h"

#define TIMESTAMP_MAX_SEQ_SIZE 8192

/* Генерация последовательностей с метками времени
 * Идея:
 * - выбрать опорную дата так, чтобы все остальные были до нее (она
 *   самая новая)
 * - вычислить индекс.
 * - сдвигать дату на 1 фиксированный интервал назад и вычислять новое значение
 *   индекса
 * - получить все необходимые пары индекс-время
 * - размеры архиовов невелики (тысячи записей), подбор будет работать
 *   довольно быстро
 *
 * Алгоритм:
 * - Перевести локальное время в UTC
 * - Сдвигать UTC времся назад с фиксированным шагом
 * - Для каждого сдвига UTC восстановить локальное время и вычислить индекс.
 * - Если индекс еще не был добавлен в последовательность,
 *   то округлить и добавить.
 * - Делать до тех пор, пока не будет достигнут требуемый размер последовательности
 *   или мы не выйдем за диапазон
 * - Не забывать, что математику со временем удобно делать в UTC, но Тэконы
 *   работают в локальном времени.
*/
struct timestamp_seq {
    int64_t time[TIMESTAMP_MAX_SEQ_SIZE];
    size_t count;
};

/* Месячная последовательность.
 * size - 12 или 48 */
int timestamp_seq_month(struct timestamp_seq * self, const struct tekon_date * date, size_t depth);

/* Суточная последовательность */
int timestamp_seq_day(struct timestamp_seq * self, const struct tekon_date * date);

/* Часовая последовательность
 * size : 384 (16 дней), 764 (32 дня), 1536 (64 дня) */
int timestamp_seq_hour(struct timestamp_seq * self, const struct tekon_date * date, const struct tekon_time * time, size_t depth);

/* Интервальная последовательность
 * interval - минуты */
int timestamp_seq_interval(struct timestamp_seq * self, const struct tekon_date * date, const struct tekon_time * time, size_t depth, size_t interval);

void timestamp_seq_init(struct timestamp_seq * self);


int64_t timestamp_seq_get(const struct timestamp_seq * self, size_t index);
size_t timestamp_seq_size(const struct timestamp_seq * self);

int tekon_time_to_local(const struct tekon_time * tekon, struct tm * local);
int tekon_time_from_local(struct tekon_time * tekon, const struct tm * local);
int tekon_date_to_local(const struct tekon_date * tekon, struct tm * local);
int tekon_date_from_local(struct tekon_date * tekon, const struct tm * local);


#ifdef __cplusplus
}
#endif

#endif
