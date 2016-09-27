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

#ifndef __JET_SPACE_H__
#define __JET_SPACE_H__

#include <asp/space.h>

#define pok_create_space ja_space_create

#define pok_space_switch ja_space_switch
#define pok_space_get_current() ja_space_get_current()

#define jet_user_space_jump ja_user_space_jump

#endif /* __JET_ASP_SPACE_H__ */
