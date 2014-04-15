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


#ifndef __POK_x86_TYPES_H__
#define __POK_X86_TYPES_H__

typedef unsigned short        uint8_t;
typedef unsigned short        uint16_t;
typedef unsigned int          uint32_t;
typedef long int              int32_t;
typedef unsigned long long    uint64_t;

typedef short                 int8_t;
typedef short                 int16_t;
typedef signed long long      int64_t;

typedef unsigned int          size_t;
typedef int		      ssize_t;
typedef unsigned long int     uintptr_t;
typedef long int              intptr_t;

#define INT8_MAX (0x7f)
#define INT8_MIN (-INT8_MAX - 1L)
#define UINT8_MAX (0xffu)

#define INT16_MAX (0x7fff)
#define INT16_MIN (-INT16_MAX - 1L)
#define UINT16_MAX (0xffffu)

#define INT32_MAX (0x7fffffff)
#define INT32_MIN (-INT32_MAX - 1L)
#define UINT32_MAX (0xffffffffu)

#endif
