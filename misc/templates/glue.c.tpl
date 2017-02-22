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

{% for i in instances%}
    #include <{{i.type}}_gen.h>
        void __{{i.type}}_init__({{i.type}}*);
        void __{{i.type}}_activity__({{i.type}}*);
        {% if i.name in port_array_dict %}
         {% for port, size in port_array_dict[i.name].iteritems() %}
            struct port_ops {{i.name}}_array_for_{{port}}[{{size + 1}}];
         {% endfor %}
        {% endif %}
        {{i.type}} {{i.name}} = {
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


void glue_main()
{
    {% for i in instances %}
            __{{i.type}}_init__(&{{i.name}});

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

void glue_activity()
{
    while (1) {
        {% for i in instances%}
                __{{i.type}}_activity__(&{{i.name}});
        {% endfor %}
    }

}
