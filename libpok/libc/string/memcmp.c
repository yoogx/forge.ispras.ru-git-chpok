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
 * Created by julien on Tue Dec  8 15:53:28 2009 
 */

#include <core/dependencies.h>
#include <types.h>

#ifdef POK_CONFIG_NEEDS_FUNC_MEMCMP

int memcmp (const void* v1, const void* v2, size_t n)
{
   const unsigned char *s1 = v1;
   const unsigned char *s2 = v2;
   size_t  i;

   for (i = 0; i < n; i++) {
      int diff = s1[i] - s2[i];
      if (diff) return diff;
   }
   return 0;
}

#endif

