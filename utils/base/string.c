/* Copyright (c) 2019
 * Alexander Shirokov
 * Schneider Electric
 * See LICENSE for details. */

#ifdef __cplusplus
extern "C" {
#endif
#include "utils/base/string.h"

#include <assert.h>
#include <ctype.h>
#include <string.h>

const char* string_trim(const char * str)
{
    assert(str);
    while(*str != '\0' &&
            isblank(*str))
        str++;
    return str;
}

const char* string_next(const char * str)
{
    return str && *str!='\0' ? str + 1 : NULL;
}

int string_is_term(const char *str)
{
    return str ? *str == '\0' : 1;
}

#ifdef __cplusplus
}
#endif




