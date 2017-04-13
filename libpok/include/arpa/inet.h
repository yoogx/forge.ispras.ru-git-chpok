/*
 * Institute for System Programming of the Russian Academy of Sciences
 * Copyright (C) 2017 ISPRAS
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, Version 3.
 *
 * This program is distributed in the hope # that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License version 3 for more details.
 */

/* Some functions from POSIX arpa/inet.h which are needed for the libjet. */

#ifndef __LIBJET_ARPA_INET_H__
#define __LIBJET_ARPA_INET_H__

#include <stdint.h>

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__

static inline uint16_t htons(uint16_t x)
{
    return __builtin_bswap16(x);
}

static inline uint16_t ntohs(uint16_t x)
{
    return __builtin_bswap16(x);
}

static inline uint32_t htonl(uint32_t x)
{
    return __builtin_bswap32(x);
}

static inline uint32_t ntohl(uint32_t x)
{
    return __builtin_bswap32(x);
}

// Extension for 64-bit integers.
static inline uint64_t hton64(uint64_t x)
{
    return __builtin_bswap64(x);
}

static inline uint64_t ntoh64(uint64_t x)
{
    return __builtin_bswap64(x);
}

#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__

static inline uint16_t htons(uint16_t x)
{
    return x;
}

static inline uint16_t ntohs(uint16_t x)
{
    return x;
}

static inline uint32_t htonl(uint32_t x)
{
    return x;
}

static inline uint32_t ntohl(uint32_t x)
{
    return x;
}

// Extension for 64-bit integers.
static inline uint64_t hton64(uint64_t x)
{
    return x;
}

static inline uint64_t ntoh64(uint64_t x)
{
    return x;
}

#else /* __BYTE_ORDER__ */
#error "Unrecognized endianness"
#endif /* __BYTE_ORDER__ */

#endif /* __LIBJET_ARPA_INET_H__ */
