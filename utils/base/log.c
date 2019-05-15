/* Copyright (c) 2019
 * Alexander Shirokov
 * Schneider Electric
 * See LICENSE for details. */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>
#include <stdio.h>

static int log_level = 1;


void log_setlevel(int level)
{
    log_level = level;
}

/* Вывод сообщений в лог. Особенности:
 * - вывод в stderr
 * - первый символ в аргументе format должен содержать уровень сообщения
 *   (LOG_ERR, LOG_WARN и т.п.). Т.о. образом вызов будет выглять сл. образом:
 *   log_print(LOG_ERR "bla-bla-bla");
 */
int log_print(const char * format, ...)
{
    char level = format[0] - '0';

    // Отфильтровать по типу сообщений
    if(level > log_level)
        return 0;

    // Вывести
    va_list args;
    va_start(args, format);
    int result = vfprintf(stderr, format + 1, args);
    va_end(args);
    return result;
}

#ifdef __cplusplus
}
#endif
