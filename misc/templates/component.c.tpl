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
  {% for i_func in interfaces[p.type].functions %}
      {{i_func.return_type}} call_{{p.name}}_{{i_func.name}}({{component.name}} *self{{args(i_func)}})
      {
         if (self->out.{{p.name}}.ops == NULL) {
             printf("WRONG CONFIG: out port {{p.name}} of component {{component.name}} was not initialized\n");
             //fatal_error?
         }
         return self->out.{{p.name}}.ops->{{i_func.name}}(self->out.{{p.name}}.owner{%for i in range(1, i_func.args_type|length)%}, arg{{i}}{%endfor%});
      }
  {% endfor %}
{% endfor %}


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
