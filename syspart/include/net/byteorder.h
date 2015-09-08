/*  
 *  Copyright (C) 2014 Maxim Malkov, ISPRAS <malkov@ispras.ru> 
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __POK_NET_BYTEORDER_H__
#define __POK_NET_BYTEORDER_H__


#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__

static inline uint16_t hton16(uint16_t x)
{
    return ((x >> 8) & 0xFF) |
           ((x & 0xFF) << 8);
}

static inline uint16_t ntoh16(uint16_t x)
{
    return hton16(x);
}

static inline uint32_t hton32(uint32_t x)
{
    return __builtin_bswap32(x);
}

static inline uint32_t ntoh32(uint32_t x)
{
    return __builtin_bswap32(x);
}


#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__

#define F(TYPE, NAME) \
    static inline TYPE NAME(TYPE x) { return x; }

F(uint16_t, hton16)
F(uint16_t, ntoh16)
F(uint32_t, ntoh32)
F(uint32_t, hton32)

#undef F

#else
    #error "unrecognized endianness"
#endif

#endif
