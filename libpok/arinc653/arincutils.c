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
 * Created by matias on Wed Apr 04 09:34:13 2012
 */

#include <config.h>

#ifdef POK_NEEDS_ARINC653_PROCESS

#include  <arinc653/arincutils.h>
#include <libc/string.h>

pok_bool_t get_process_id(PROCESS_NAME_TYPE process_name, pok_thread_id_t *id)
{
    pok_thread_id_t i;
    for (i = 0; i < POK_CONFIG_NB_THREADS; i++) {
        if (strcmp(process_name, arinc_process_attribute[i].NAME) == 0) {
            if (id != NULL) {
                *id = i;
            }
            return TRUE;
        }
    }
    return FALSE;
}

#endif
