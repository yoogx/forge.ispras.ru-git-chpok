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

#ifdef POK_NEEDS_NETWORKING_VIRTIO
#ifndef __POK_DRIVERS_VIRTIO_VIRTIO_NETWORK_H__
#define __POK_DRIVERS_VIRTIO_VIRTIO_NETWORK_H__

#include <net/network.h>

extern pok_network_driver_device_t pok_network_virtio_device;

#endif // __POK_DRIVERS_VIRTIO_VIRTIO_NETWORK_H__
#endif // POK_NEEDS_NETWORKING_VIRTIO
