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

#if defined (POK_NEEDS_DEBUG) || defined (POK_NEEDS_PORTS_QUEUEING) || defined (POK_NEEDS_PORTS_SAMPLING)

#include <libc.h>

__attribute__ ((weak))
int strncasecmp(const char *s1, const char *s2, size_t size)
{
  unsigned int i;
  for (i = 0; i < size; i++)
  { 
      if (s1[i] == '\0' && s2[i] == '\0')
          return 0;
      if ((s1[i]>='a' && s1[i]<='z')&&(s2[i]<'a' || s2[i]>'z'))
      {
          if (s1[i]+'A' - 'a' < s2[i])
                return -1;
          if (s1[i]+'A' - 'a' > s2[i])
                return 1;
      }else
          if ((s2[i]>='a' && s2[i]<='z')&&(s1[i]<'a' || s1[i]>'z'))
          {    if (s1[i] < s2[i]+'A' - 'a')
                  return -1;
              if (s1[i] > s2[i]+'A' - 'a')
                  return 1;
      }
      else
      {
              if (s1[i] < s2[i])
                  return -1;
              if (s1[i] > s2[i])
                  return 1;
      }
  }
  return 0;
}
#endif
