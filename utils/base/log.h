/* Copyright (c) 2019
 * Alexander Shirokov
 * Schneider Electric
 * See LICENSE for details. */

#ifndef UTILS_BASE_LOG_H
#define UTILS_BASE_LOG_H

#ifdef __cplusplus
extern "C" {
#endif

#define LOG_ERR "1"
#define LOG_WARN "2"
#define LOG_INFO "3"

// Уровень логирования
void log_setlevel(int level);

// Вывод в лог.
int log_print(const char * format, ...);

#ifdef __cplusplus
}
#endif

#endif
