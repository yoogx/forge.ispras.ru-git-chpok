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
 * This file also incorporates work covered by the following 
 * copyright and license notice:
 *
 *  Copyright (C) 2013-2014 Maxim Malkov, ISPRAS <malkov@ispras.ru> 
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
 *
 * Created by julien on Thu Jan 15 23:34:13 2009 
 */


#ifndef __POK_USER_BUFFER_H__
#define __POK_USER_BUFFER_H__

#include <config.h>

#ifdef POK_NEEDS_BUFFERS

#include <errno.h>

// must be at least MAX_NAME_LENGTH of ARINC653 (which is 30)
#define POK_BUFFER_MAX_NAME_LENGTH 30

/* 
 * This essentially mirrors ARINC-653 BUFFER_STATUS type.
 */
typedef struct {
   pok_range_t          nb_messages;
   pok_range_t          max_messages;
   pok_size_t           message_size;
   pok_range_t          waiting_processes;
} pok_buffer_status_t;


// All buffer-related functions are already defined as syscalls.
#include <core/syscall.h>

#endif

#endif
