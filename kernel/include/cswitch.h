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

#ifndef __JET_CSWITCH_H__
#define __JET_CSWITCH_H__

#include <asp/cswitch.h>

#define jet_context_init ja_context_init
#define jet_context_switch ja_context_switch
#define jet_context_jump ja_context_jump
#define jet_context_restart ja_context_restart
#define jet_context_restart_and_save ja_context_restart_and_save

#endif /* __JET_CSWITCH_H__ */
