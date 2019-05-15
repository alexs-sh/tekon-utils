/* Copyright (c) 2019
 * Alexander Shirokov
 * Schneider Electric
 * See LICENSE for details. */

#ifndef UTILS_BASE_STRING_H
#define UTILS_BASE_STRING_H

#ifdef __cplusplus
extern "C" {
#endif

const char* string_trim(const char * str);
const char* string_next(const char * str);
int string_is_term(const char *str);

#ifdef __cplusplus
}
#endif

#endif
