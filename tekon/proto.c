/* Copyright (c) 2019
 * Alexander Shirokov
 * Schneider Electric
 * See LICENSE for details. */

#ifdef __cplusplus
extern "C" {
#endif
#include <assert.h>
#include "tekon/proto.h"

uint8_t tekon_crc(const void * buffer, size_t size)
{
    assert(buffer);
    assert(size);

    uint8_t crc = 0;
    const uint8_t * ptr = buffer;
    const uint8_t * end = ptr + size;
    while (ptr != end) {
        crc += *ptr++;
    }
    return crc;
}

uint8_t tekon_fixed_crc(const void * buffer, size_t size)
{
    assert(size >= 8);
    const uint8_t * ptr = buffer;
    return tekon_crc(ptr + 1, 6);
}

uint8_t tekon_variable_crc(const void * buffer, size_t size)
{
    assert(size >= 8);
    const uint8_t * ptr = buffer;
    return tekon_crc(ptr + 4, size - 6);
}



#ifdef __cplusplus
}
#endif
