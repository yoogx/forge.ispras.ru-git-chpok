/*
 *  Copyright (C) 2014 Maxim Malkov, ISPRAS <malkov@ispras.ru> 
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

#include <config.h>

#if defined (POK_NEEDS_PORTS_SAMPLING) || defined (POK_NEEDS_PORTS_QUEUEING)
#include <types.h>
#include <middleware/port.h>

static void pok_port_queueing_reset(pok_partition_id_t partid)
{
#ifdef POK_NEEDS_PORTS_QUEUEING
    int i;
    for (i = 0; i < POK_CONFIG_NB_QUEUEING_PORTS; i++) {
        if (pok_queueing_ports[i].header.partition != partid) continue;

        pok_queueing_ports[i].header.created = FALSE;
    }
#else
    (void) partid;
#endif
}   

static void pok_port_sampling_reset(pok_partition_id_t partid)
{
#ifdef POK_NEEDS_PORTS_SAMPLING
    int i;
    for (i = 0; i < POK_CONFIG_NB_SAMPLING_PORTS; i++) {
        if (pok_sampling_ports[i].header.partition != partid) continue;
        
        pok_sampling_ports[i].header.created = FALSE;
    }
#else
    (void) partid;
#endif
}   

void pok_port_reset(pok_partition_id_t part)
{
    pok_port_queueing_reset(part);
    pok_port_sampling_reset(part);
}

#endif
