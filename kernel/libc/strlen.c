/*
 *                               POK header
 * 
 * The following file is a part of the POK project. Any modification should
 * made according to the POK licence. You CANNOT use this file or a part of
 * this file is this part of a file for your own project
 *
 * For more information on the POK licence, please see our LICENCE FILE
 *
 * Please follow the coding guidelines described in doc/CODING_GUIDELINES
 *
 *                                      Copyright (c) 2007-2009 POK team 
 *
 * Created by julien on Thu Jan 15 23:34:13 2009 
 */

#include <config.h>
#include <libc.h>

#ifdef POK_NEEDS_DEBUG

int strlen (const char* str)
{
   int i;

   i = 0;

   while (*str != '\0')
   {
      str++;
      i++;
   }

   return i;
}

// TODO: move into own file?
int strnlen (const char* str, size_t n)
{
   int i;

   i = 0;

   while (i < (int)n && *str != '\0')
   {
      str++;
      i++;
   }

   return i;
}

// TODO: move into own file?
char* strncpy(char* dest, const char* src, size_t n)
{
   int m = strnlen(src, n);

   if(m < n) m++;
   
   memcpy(dest, src, m);
   
   return dest;
}

#endif

