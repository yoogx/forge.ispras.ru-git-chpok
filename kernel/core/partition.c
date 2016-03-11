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

/**
 * \file partition.c
 * \brief This file provides functions for partitioning services
 * \author Julien Delange
 *
 * The definition of useful structures can be found in partition.h
 * header file. To enable partitioning services, you must set the
 * POK_NEEDS_PARTITIONS macro.
 */

#include <config.h>

#include <arch.h>
#include <bsp.h>
#include <errno.h>
#include <dependencies.h>
#include <core/sched.h>
#include <core/error.h>
#include <core/debug.h>
#include <core/thread.h>
#include <core/partition.h>
#include <core/instrumentation.h>
#include <core/time.h>

#include <libc.h>
#include <assert.h>

void pok_partition_reset(pok_partition_t* part)
{
    part->sp = 0;
}
