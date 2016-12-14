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

#include <config.h>

#if defined (POK_NEEDS_CONSOLE) || defined (POK_NEEDS_DEBUG) || defined (POK_NEEDS_INSTRUMENTATION) || defined (POK_NEEDS_COVERAGE_INFOS)

#include <errno.h>
#include <cons.h>
#include <core/uaccess.h>

static void iostream_init(struct jet_iostream* stream)
{
   if(stream->init)
      stream->init();
}

static size_t iostream_write_null(const char* s, size_t length)
{
   return length;
}

static size_t iostream_read_null(char* s, size_t length)
{
   return 0;
}

struct jet_iostream jet_stream_null =
{
   .write = &iostream_write_null,
   .read = &iostream_read_null
};

struct jet_console jet_console_main;
struct jet_console jet_console_debug;

static struct jet_iostream* iostream_get_default(struct jet_iostream* arch_default)
{
   if(arch_default) return arch_default;
   else return &jet_stream_null;
}

void jet_console_init_all(void)
{
   jet_console_main.read_stream = iostream_get_default(ja_stream_default_read);
   jet_console_main.write_stream = iostream_get_default(ja_stream_default_write);
   jet_console_debug.read_stream = iostream_get_default(ja_stream_default_read_debug);
   jet_console_debug.write_stream = iostream_get_default(ja_stream_default_write_debug);

   iostream_init(jet_console_main.read_stream);
   iostream_init(jet_console_main.write_stream);
   iostream_init(jet_console_debug.read_stream);
   iostream_init(jet_console_debug.write_stream);
}

static size_t jet_console_read_common(struct jet_console* console, char* s, size_t length)
{
   size_t res = 0;
   size_t length_rest = length;
   char* s_rest = s;

   while(length_rest > 0)
   {
      size_t bytes = console->read_stream->read(s_rest, length_rest);
      if(!bytes) break;

      res+= bytes;
      length_rest -= bytes;
      s_rest += bytes;
   }

   return res;
}
size_t jet_console_read(char* s, size_t length)
{
   return jet_console_read_common(&jet_console_main, s, length);
}
size_t jet_console_read_debug(char* s, size_t length)
{
   return jet_console_read_common(&jet_console_debug, s, length);
}

size_t jet_console_write_common(struct jet_console* console,
   const char* s, size_t length)
{
   size_t res = 0;
   size_t length_rest = length;
   const char* s_rest = s;

   while(length_rest > 0)
   {
      size_t bytes = console->write_stream->write(s_rest, length_rest);

      res+= bytes;
      length_rest -= bytes;
      s_rest += bytes;
   }

   return res;
}

size_t jet_console_write(const char* s, size_t length)
{
   return jet_console_write_common(&jet_console_main, s, length);
}

size_t jet_console_write_debug(const char* s, size_t length)
{
   return jet_console_write_common(&jet_console_debug, s, length);
}


pok_ret_t jet_console_write_user(const char* __user s, size_t length)
{
   if(length != 0) {
      const char* __kuser k_s = jet_user_to_kernel_ro(s, length);
      if(!k_s) return POK_ERRNO_EFAULT;

      jet_console_write(k_s, length);
   }
   return POK_ERRNO_OK;
}


// Functions for backward compatibility.
pok_bool_t pok_cons_write (const char* s,
                       size_t length)
{
   jet_console_write(s, length);

   return TRUE;
}
pok_bool_t pok_cons_write_1 (const char* s,
                       size_t length)
{
   jet_console_write_debug(s, length);

   return TRUE;
}

/*
 * Contains character which has been read in 'data_to_read_*'
 *
 * If no character has been read in the last `data_to_read_*` call,
 * or character is already consumed by `read_serial_*`, this is -1.
 */
static int char_to_read_main = -1;
static int char_to_read_debug = -1;

int data_to_read_0(void)
{
   if(char_to_read_main != -1) return 1;

   char ch;
   if(jet_console_read(&ch, 1))
   {
      char_to_read_main = ch;
      return 1;
   }

   return 0;
}

int read_serial_0(void)
{
   int res = data_to_read_0();
   if(res != -1)
   {
      res = char_to_read_main;
      char_to_read_main = -1;
   }

   return res;
}

int data_to_read_1(void)
{
   if(char_to_read_debug != -1) return 1;

   char ch;
   if(jet_console_read_debug(&ch, 1))
   {
      char_to_read_debug = ch;
      return 1;
   }

   return 0;
}
int read_serial_1(void)
{
   int res = data_to_read_1();
   if(res != -1)
   {
      res = char_to_read_debug;
      char_to_read_debug = -1;
   }

   return res;
}

#endif /* POK_NEEDS_CONSOLE, POK_NEEDS_DEBUG */
