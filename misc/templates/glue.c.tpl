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

struct port_ops{
    void *ops;
    void *owner;
};

{% for comp in components%}
    #include <{{comp.type}}_gen.h>
    {% for i in comp.instances %}
        void __{{comp.type}}_init__({{comp.type}}*);
        void __{{comp.type}}_activity__({{comp.type}}*);
        {% if i.name in port_array_dict %}
         {% for port, size in port_array_dict[i.name].iteritems() %}
            struct port_ops {{i.name}}_array_for_{{port}}[{{size + 1}}];
         {% endfor %}
        {% endif %}
        {{comp.type}} {{i.name}} = {
            {% if i.state %}
            .state = {
              {% for name, val in i.state.iteritems()%}
                .{{name}} = {{val}},
              {% endfor %}
            },
            {% endif %}

            {% if i.name in port_array_dict %}
            .out = {
             {% for port, size in port_array_dict[i.name].iteritems() %}
                .{{port}} = (void *){{i.name}}_array_for_{{port}},
             {% endfor %}
            }
            {% endif %}
        };
    {% endfor %}

{% endfor %}


void __components_init__()
{
    {% for comp in components%}
        {% for i in comp.instances %}
            __{{comp.type}}_init__(&{{i.name}});
        {% endfor %}

    {% endfor %}

    {% for l in links %}
      {% if l.from.array_index is defined%}
        {{l.from.instance}}.out.{{l.from.port}}[{{l.from.array_index}}].ops = &{{l.to.instance}}.in.{{l.to.port}}.ops;
        {{l.from.instance}}.out.{{l.from.port}}[{{l.from.array_index}}].owner = &{{l.to.instance}};
      {% else %}
        {{l.from.instance}}.out.{{l.from.port}}.ops = &{{l.to.instance}}.in.{{l.to.port}}.ops;
        {{l.from.instance}}.out.{{l.from.port}}.owner = &{{l.to.instance}};
      {% endif %}
    {% endfor %}

}

void __components_activity__()
{
    while (1) {
        {% for comp in components%}
            {% for i in comp.instances %}
                __{{comp.type}}_activity__(&{{i.name}});
            {% endfor %}
        {% endfor %}
    }

}
