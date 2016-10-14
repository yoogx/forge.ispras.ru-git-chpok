/*
 * Institute for System Programming of the Russian Academy of Sciences
 * Copyright (C) 2016 ISPRAS
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, Version 3.
 *
 * This program is distributed in the hope # that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License version 3 for more details.
 */

#include <arinc_config.h>

#ifdef POK_NEEDS_ARINC653_BUFFER
// Maximum number of buffers.
size_t arinc_config_nbuffers = {{part.num_arinc653_buffers}};
#endif /* POK_NEEDS_ARINC653_BUFFER */

#ifdef POK_NEEDS_ARINC653_BLACKBOARD
// Maximum number of blackboards.
size_t arinc_config_nblackboards = {{part.num_arinc653_blackboards}};
#endif /* POK_NEEDS_ARINC653_BLACKBOARD */

#ifdef POK_NEEDS_ARINC653_SEMAPHORE
// Maximum number of semaphores.
size_t arinc_config_nsemaphores = {{part.num_arinc653_semaphores}};
#endif /* POK_NEEDS_ARINC653_SEMAPHORE */

#ifdef POK_NEEDS_ARINC653_EVENT
// Maximum number of events.
size_t arinc_config_nevents = {{part.num_arinc653_events}};
#endif /* POK_NEEDS_ARINC653_EVENT */

#if defined(POK_NEEDS_ARINC653_BUFFER) || defined(POK_NEEDS_ARINC653_BLACKBOARD)
// Memory for messages, used by buffers and blackboards.
size_t arinc_config_messages_memory_size = {{part.buffer_data_size + part.blackboard_data_size}};
#endif /* defined(POK_NEEDS_ARINC653_BUFFER) || defined(POK_NEEDS_ARINC653_BLACKBOARD) */

{%if part.is_system%}
{% include 'deployment_user_system'%}
{%endif%}
