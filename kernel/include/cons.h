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

#ifndef __JET_CONS_H__
#define __JET_CONS_H__

#include <types.h>
#include <asp/cons.h>
#include <common.h>
#include <errno.h>

/* 
 * Do-nothing stream.
 * 
 * Everything which is written to it disappears.
 * 
 * Reading from it always returns 0.
 * 
 * Exposed for future use.
 */
extern struct jet_iostream jet_stream_null;

/* Console which incorporates write and read streams. */
struct jet_console
{
    /* Stream for write. Not NULL. */
    struct jet_iostream* write_stream;
    /* Stream for read. Not NULL. */
    struct jet_iostream* read_stream;
};

/* 
 * Main and debug consoles.
 * 
 * Exposed for future use.
 */
extern struct jet_console jet_console_main;
extern struct jet_console jet_console_debug;

/* 
 * Read characters from console until 'length' characters has been read
 * or none characters are left in the console.
 */
size_t jet_console_read(char* s, size_t length);

/*
 * Write characters into console until all of them will be written.
 * 
 * Currently always returns 'length'.
 */
size_t jet_console_write(const char* s, size_t length);

/* 
 * Read characters from debug console until 'length' characters has been read
 * or none characters are left in the console.
 */
size_t jet_console_read_debug(char* s, size_t length);

/*
 * Write characters into debug console until all of them will be written.
 * 
 * Currently always returns 'length'.
 */
size_t jet_console_write_debug(const char* s, size_t length);


/* Syscall for write into main console from user space. */
pok_ret_t jet_console_write_user(const char* __user s, size_t length);


// Functions for backward compatibility. TODO: Remove them and their usage.
pok_bool_t pok_cons_write (const char* s,
                       size_t length);
pok_bool_t pok_cons_write_1 (const char* s,
                       size_t length);

int data_to_read_0(void);
int read_serial_0(void);
int data_to_read_1(void);
int read_serial_1(void);

#endif /* __JET_CONS_H__ */
