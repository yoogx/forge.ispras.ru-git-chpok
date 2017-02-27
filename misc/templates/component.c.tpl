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
#include "{{component.name}}_gen.h"

{% macro args(func) %}
{% for i in range(1, func.args_type|length) %}, {{func.args_type[i]}} arg{{i}}{%endfor%}
{% endmacro %}


{% for p in component.in_ports %}
  {% for i_func in interfaces[p.type].functions %}
    static {{i_func.return_type}} __wrapper_{{p.implementation[i_func.name]}}(self_t *arg0{{args(i_func)}})
    {
        return {{p.implementation[i_func.name]}}(({{component.name}}*) arg0{%for i in range(1, i_func.args_type|length)%}, arg{{i}}{%endfor%});
    }

  {% endfor %}
{% endfor %}


{% for p in component.out_ports %}
 {% if p.is_array %}
  {% for i_func in interfaces[p.type].functions %}
      {{i_func.return_type}} {{component.name}}_call_{{p.name}}_{{i_func.name}}_by_index(int idx, {{component.name}} *self{{args(i_func)}})
      {
         if (self->out.{{p.name}}[idx].ops == NULL) {
             printf("WRONG CONFIG: out port {{p.name}} of component {{component.name}} was not initialized\n");
             //fatal_error?
         }
         return self->out.{{p.name}}[idx].ops->{{i_func.name}}(self->out.{{p.name}}[idx].owner{%for i in range(1, i_func.args_type|length)%}, arg{{i}}{%endfor%});
      }
  {% endfor %}
 {% else %}
  {% for i_func in interfaces[p.type].functions %}
      {{i_func.return_type}} {{component.name}}_call_{{p.name}}_{{i_func.name}}({{component.name}} *self{{args(i_func)}})
      {
         if (self->out.{{p.name}}.ops == NULL) {
             printf("WRONG CONFIG: out port {{p.name}} of component {{component.name}} was not initialized\n");
             //fatal_error?
         }
         return self->out.{{p.name}}.ops->{{i_func.name}}(self->out.{{p.name}}.owner{%for i in range(1, i_func.args_type|length)%}, arg{{i}}{%endfor%});
      }
  {% endfor %}
 {% endif %}
{% endfor %}

{% if has_per_instance_memory_block %}
 pok_ret_t {{component.name}}_get_memory_block_status(
         {{component.name}} *self,
         const char *name,
         jet_memory_block_status_t *mb_status)
 {
     char full_name[30]; //use MAX_NAME_LENGTH instead??
     snprintf(full_name, 30, "%s_%s", self->instance_name, name);
     return jet_memory_block_get_status(full_name, mb_status);
 }
{% endif %}

void __{{component.name}}_init__({{component.name}} *self)
{
    {% for p in component.in_ports %}
        {% for i_func, comp_func in p.implementation.iteritems() %}
            self->in.{{p.name}}.ops.{{i_func}} = __wrapper_{{comp_func}};
        {% endfor %}
    {% endfor %}

    {% if component.init_func %}
        {{component.init_func}}(self);
    {% endif %}
}

void __{{component.name}}_activity__({{component.name}} *self)
{
    {% if component.activity %}
        {{component.activity}}(self);
    {% endif %}
}
