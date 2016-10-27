#ifndef __INTERFACES_{{interface.name|upper}}_H__
#define __INTERFACES_{{interface.name|upper}}_H__

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


#include <lib/common.h>
{% for add_h in interface.additional_h_files %}
    #include {{add_h}}
{% endfor %}

typedef struct {
  {% for func in interface.functions %}
    {{func.return_type}} (*{{func.name}})({{func.args_type|join(', ') }});
    {% endfor %}
} {{interface.name}};


#endif

