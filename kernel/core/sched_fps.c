 /*
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
 */


/*
 * Basic fixed-priority pre-emptive scheduler.
 */

#include <core/sched.h>
#include <core/sched_fps.h>
#include <core/thread.h>
#include <core/partition.h>

#include <libc.h>

// returns true if a > b (greater priority)
static pok_bool_t compare_threads(pok_thread_t *a, pok_thread_t *b)
{
    if (a->priority != b->priority) {
        return a->priority > b->priority;
    }
    // TODO compare last sched time?
    return FALSE;
}

void fps_initialize(void) {

}

pok_thread_id_t fps_elect_thread(void) {
    // find the best thread
    // TODO it scans linearly right now (perhaps, heap queue will be better?)
    pok_thread_id_t i, best_thread_idx;
    pok_bool_t found_any = FALSE;
    for (i = POK_CURRENT_PARTITION.thread_index_low; i < POK_CURRENT_PARTITION.thread_index; i++) {
        pok_thread_t *thread = &pok_threads[i]; 
        if (pok_thread_is_runnable(thread)) {
            if (found_any) {
                if (compare_threads(thread, &pok_threads[best_thread_idx])) {
                    best_thread_idx = i;
                }
            } else {
                best_thread_idx = i;
                found_any = TRUE;
            }
        }
    }

    if (found_any) {
        return best_thread_idx;
    } else {
        return IDLE_THREAD;
    }
}

void fps_enqueue_thread(pok_thread_id_t thread_id) {
    (void) thread_id;
}

const pok_scheduler_ops pok_fps_scheduler_ops = {
    .initialize = fps_initialize,
    .elect_thread = fps_elect_thread,
    .enqueue_thread = fps_enqueue_thread 
};
