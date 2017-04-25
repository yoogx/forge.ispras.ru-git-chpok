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

#include <config.h>

#include <core/thread.h>
#include <arinc653/arincutils.h>
#include <arinc653/types.h>
#include <arinc653/process.h>
#include <string.h>
#include <utils.h>
#include <core/partition.h>

#include <kernel_shared_data.h>
#include "map_error.h"

void GET_PROCESS_ID(
    PROCESS_NAME_TYPE process_name,
    PROCESS_ID_TYPE   *process_id,
    RETURN_CODE_TYPE  *RETURN_CODE)
{
    pok_thread_id_t id;
    jet_ret_t core_ret;

    core_ret = pok_thread_find(process_name, &id);

    MAP_ERROR_BEGIN(core_ret)
        MAP_ERROR(EOK, NO_ERROR);
        MAP_ERROR(JET_INVALID_CONFIG, INVALID_CONFIG);
        MAP_ERROR_EFAULT();
        MAP_ERROR_DEFAULT();
    MAP_ERROR_END()

    if (core_ret == EOK)
        *process_id = id + 1;
}

void GET_MY_ID (PROCESS_ID_TYPE   *process_id,
		RETURN_CODE_TYPE  *RETURN_CODE )
{
    if(kshd->partition_mode != POK_PARTITION_MODE_NORMAL)
    {
        // Main thread has no id.
        *RETURN_CODE = INVALID_MODE;
    }
    else if(kshd->current_thread_id == kshd->error_thread_id)
    {
        // Error thread has no id.
        *RETURN_CODE = INVALID_MODE;
    }
    else
    {
        *process_id = kshd->current_thread_id + 1;
        *RETURN_CODE = NO_ERROR;
    }
}

void GET_PROCESS_STATUS (
    PROCESS_ID_TYPE     process_id,
    PROCESS_STATUS_TYPE *process_status,
    RETURN_CODE_TYPE    *RETURN_CODE)
{
    pok_thread_status_t status;
    jet_ret_t           core_ret;

    core_ret = pok_thread_get_status(process_id - 1,
        process_status->ATTRIBUTES.NAME, &process_status->ATTRIBUTES.ENTRY_POINT,
        &status);

    MAP_ERROR_BEGIN (core_ret)
        MAP_ERROR(EOK, NO_ERROR);
        MAP_ERROR_EFAULT();
        MAP_ERROR_DEFAULT();
    MAP_ERROR_END()

    if (core_ret != EOK) {
        return;
    }

    process_status->DEADLINE_TIME = status.deadline_time;
#define MAP_STATUS(from, to) case (from): process_status->PROCESS_STATE = (to); break
    switch (status.state) {
        MAP_STATUS(POK_STATE_STOPPED, DORMANT);
        MAP_STATUS(POK_STATE_RUNNABLE, READY);
        MAP_STATUS(POK_STATE_WAITING, WAITING);
        MAP_STATUS(POK_STATE_RUNNING, RUNNING);
    }
#undef MAP_STATUS
    process_status->ATTRIBUTES.BASE_PRIORITY = status.attributes.priority;
    process_status->ATTRIBUTES.DEADLINE =
        (status.attributes.deadline == DEADLINE_SOFT)
            ? SOFT : HARD;
    process_status->CURRENT_PRIORITY = status.current_priority;
    process_status->ATTRIBUTES.PERIOD = status.attributes.period;
    process_status->ATTRIBUTES.TIME_CAPACITY = status.attributes.time_capacity;
    process_status->ATTRIBUTES.STACK_SIZE = status.attributes.stack_size;
    process_status->DEADLINE_TIME = status.deadline_time;
}

void CREATE_PROCESS (
    PROCESS_ATTRIBUTE_TYPE  *attributes,
    PROCESS_ID_TYPE         *process_id,
    RETURN_CODE_TYPE        *RETURN_CODE)
{
    pok_thread_attr_t core_attr;
    jet_ret_t         core_ret;
    pok_thread_id_t   core_process_id;

    if (attributes->NAME[0] == '\0') {
        // empty names are not allowed
        *RETURN_CODE = INVALID_PARAM;
        return;
    }

    core_attr.priority        = (uint8_t) attributes->BASE_PRIORITY;
    core_attr.period          = attributes->PERIOD;
    if (attributes->DEADLINE == SOFT) {
        core_attr.deadline = DEADLINE_SOFT;
    } else if (attributes->DEADLINE == HARD) {
        core_attr.deadline = DEADLINE_HARD;
    } else {
        *RETURN_CODE = INVALID_PARAM;
        return;
    }
    core_attr.time_capacity   = attributes->TIME_CAPACITY;
    core_attr.stack_size      = attributes->STACK_SIZE;
    core_ret = pok_thread_create (attributes->NAME, attributes->ENTRY_POINT,
        &core_attr, &core_process_id);
    MAP_ERROR_BEGIN(core_ret)
        MAP_ERROR(EOK, NO_ERROR);
        MAP_ERROR(EEXIST, NO_ACTION);
        MAP_ERROR(EINVAL, INVALID_PARAM);
        MAP_ERROR(JET_INVALID_CONFIG, INVALID_CONFIG);
        MAP_ERROR(JET_INVALID_MODE, INVALID_MODE);
        MAP_ERROR_EFAULT();
        MAP_ERROR_DEFAULT();
    MAP_ERROR_END()

    if (core_ret != EOK) return;

    kshd->tshd[core_process_id].private_data = NULL;
    *process_id = core_process_id + 1;
}

void STOP_SELF ()
{
    pok_thread_stop ();
}


void SET_PRIORITY (
    PROCESS_ID_TYPE  process_id,
    PRIORITY_TYPE    priority,
    RETURN_CODE_TYPE *RETURN_CODE)
{
    if (priority < MIN_PRIORITY_VALUE || priority > MAX_PRIORITY_VALUE) {
        *RETURN_CODE = INVALID_PARAM;
        return;
    }

    jet_ret_t core_ret = pok_thread_set_priority(process_id - 1, priority);
    MAP_ERROR_BEGIN(core_ret)
        MAP_ERROR(EOK, NO_ERROR);
        MAP_ERROR(EINVAL, INVALID_PARAM);
        MAP_ERROR(JET_INVALID_MODE, INVALID_MODE);
        MAP_ERROR_DEFAULT();
    MAP_ERROR_END()
}

void SUSPEND_SELF (
    SYSTEM_TIME_TYPE time_out,
    RETURN_CODE_TYPE *RETURN_CODE)
{
    jet_ret_t core_ret = pok_thread_suspend(&time_out);

    MAP_ERROR_BEGIN(core_ret)
        MAP_ERROR(EOK, NO_ERROR);
        MAP_ERROR(ETIMEDOUT, TIMED_OUT);
        MAP_ERROR(JET_INVALID_MODE, INVALID_MODE);
        MAP_ERROR(EINVAL, INVALID_PARAM);
        // EFAULT is impossible
        MAP_ERROR_DEFAULT();
    MAP_ERROR_END()
}

void SUSPEND (
    PROCESS_ID_TYPE     process_id,
    RETURN_CODE_TYPE    *RETURN_CODE)
{
    jet_ret_t core_ret = pok_thread_suspend_target(process_id - 1);

    MAP_ERROR_BEGIN(core_ret)
        MAP_ERROR(EOK, NO_ERROR);
        MAP_ERROR(EINVAL, INVALID_PARAM);
        MAP_ERROR(JET_INVALID_MODE_TARGET, INVALID_MODE);
        MAP_ERROR(JET_NOACTION, NO_ACTION);
        MAP_ERROR_DEFAULT();
    MAP_ERROR_END()
}

void RESUME (
    PROCESS_ID_TYPE     process_id,
    RETURN_CODE_TYPE    *RETURN_CODE)
{
    jet_ret_t core_ret = pok_thread_resume(process_id - 1);

    MAP_ERROR_BEGIN(core_ret)
        MAP_ERROR(EOK, NO_ERROR);
        MAP_ERROR(EINVAL, INVALID_PARAM);
        MAP_ERROR(JET_INVALID_MODE_TARGET, INVALID_MODE);
        MAP_ERROR(JET_NOACTION, NO_ACTION);
        MAP_ERROR_DEFAULT();
    MAP_ERROR_END()
}

void START (
    PROCESS_ID_TYPE     process_id,
    RETURN_CODE_TYPE    *RETURN_CODE)
{
    DELAYED_START(process_id, 0, RETURN_CODE);
}

void STOP(
    PROCESS_ID_TYPE     process_id,
    RETURN_CODE_TYPE    *RETURN_CODE)
{
    jet_ret_t core_ret = pok_thread_stop_target(process_id - 1);

    MAP_ERROR_BEGIN (core_ret)
        MAP_ERROR(EOK, NO_ERROR);
        MAP_ERROR(EINVAL, INVALID_PARAM);
        MAP_ERROR(JET_NOACTION, NO_ACTION);
        MAP_ERROR_DEFAULT();
    MAP_ERROR_END()
}

void DELAYED_START(
    PROCESS_ID_TYPE   process_id,
    SYSTEM_TIME_TYPE  delay_time,
    RETURN_CODE_TYPE *RETURN_CODE)
{
    jet_ret_t core_ret = pok_thread_delayed_start(process_id - 1, &delay_time);

    MAP_ERROR_BEGIN (core_ret)
        MAP_ERROR(EOK, NO_ERROR);
        MAP_ERROR(EINVAL, INVALID_PARAM);
        // EFAULT is impossible
        MAP_ERROR(JET_NOACTION, NO_ACTION);
        MAP_ERROR(JET_INVALID_MODE_TARGET, INVALID_MODE); //[IPPC]
        MAP_ERROR_DEFAULT();
    MAP_ERROR_END()
}

void LOCK_PREEMPTION (LOCK_LEVEL_TYPE *LOCK_LEVEL, RETURN_CODE_TYPE *RETURN_CODE)
{
    jet_ret_t core_ret = pok_partition_inc_lock_level(LOCK_LEVEL);

    MAP_ERROR_BEGIN(core_ret)
        MAP_ERROR(EOK, NO_ERROR);
        MAP_ERROR(JET_NOACTION, NO_ACTION);
        MAP_ERROR(JET_INVALID_CONFIG, INVALID_CONFIG);
        MAP_ERROR_EFAULT();
        MAP_ERROR_DEFAULT();
    MAP_ERROR_END()
}

void UNLOCK_PREEMPTION (LOCK_LEVEL_TYPE *LOCK_LEVEL, RETURN_CODE_TYPE *RETURN_CODE)
{
    jet_ret_t core_ret = pok_partition_dec_lock_level(LOCK_LEVEL);

    MAP_ERROR_BEGIN(core_ret)
        MAP_ERROR(EOK, NO_ERROR);
        MAP_ERROR(JET_NOACTION, NO_ACTION);
        MAP_ERROR_EFAULT();
        MAP_ERROR_DEFAULT();
    MAP_ERROR_END()
}
