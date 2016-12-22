/*
 * Institute for System Programming of the Russian Academy of Sciences
 * Copyright (C) 2016 ISPRAS
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

#include <libc.h>
#include <assert.h>
#include <asp/cons.h>

static size_t iostream_write_common(const char* s, size_t length, int flag)
{
    assert(0);
    return 0;
}


static size_t iostream_read_common(char* s, size_t length, int flag)
{
    assert(0);
    return 0;
}

static size_t iostream_write_main(const char* s, size_t length)
{
    return iostream_write_common(s, length, 0);
}
static size_t iostream_write_debug(const char* s, size_t length)
{
    return iostream_write_common(s, length, 1);
}
static size_t iostream_read_main(char* s, size_t length)
{
    return iostream_read_common(s, length, 0);
}
static size_t iostream_read_debug(char* s, size_t length)
{
    return iostream_read_common(s, length, 1);
}

struct jet_iostream arm_stream_main =
{
    .write = &iostream_write_main,
    .read  = &iostream_read_main
};
struct jet_iostream arm_stream_debug =
{
    .write = &iostream_write_debug,
    .read  = &iostream_read_debug
};

struct jet_iostream* ja_stream_default_read = &arm_stream_main;
struct jet_iostream* ja_stream_default_write = &arm_stream_main;
struct jet_iostream* ja_stream_default_read_debug = &arm_stream_debug;
struct jet_iostream* ja_stream_default_write_debug = &arm_stream_debug;
