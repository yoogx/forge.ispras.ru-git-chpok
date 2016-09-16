#ifndef __{{component.name}}_GEN_H__
#define __{{component.name}}_GEN_H__

{% for h in component.additional_h_files %}
    #include {{h}}
{% endfor %}

{% for p in component.in_ports %}
    #include <interfaces/{{p.type}}_gen.h>
{% endfor %}

{% for p in component.out_ports %}
    #include <interfaces/{{p.type}}_gen.h>
{% endfor %}

typedef struct {{component.name}}_state {
  {%for name, type in component.state_struct.iteritems()%}
    {{type}} {{name}};
  {% endfor %}
}{{component.name}}_state;

typedef struct {
    {{component.name}}_state state;
    struct {
        {% for p in component.in_ports %}
            struct {
                {{p.type}} ops;
            } {{p.name}};
        {% endfor %}
    } in;
    struct {
        {% for p in component.out_ports %}
            struct {
                {{p.type}} *ops;
                self_t *owner;
            } {{p.name}};
        {% endfor %}
    } out;
} {{component.name}};


{% macro args(func)%}
    {% for arg in func.args_type[1:]%}, {{arg}}{%endfor%}
{% endmacro %}

{% for p in component.in_ports %}
  {% for i_func in interfaces[p.type].functions %}
      {{i_func.return_type}} {{p.implementation[i_func.name]}}({{component.name}} *{{args(i_func)}});
  {% endfor %}
{% endfor %}

{% for p in component.out_ports %}
  {% for i_func in interfaces[p.type].functions %}
      {{i_func.return_type}} call_{{p.name}}_{{i_func.name}}({{component.name}} *{{args(i_func)}});
  {% endfor %}
{% endfor %}



{% if component.init_func %}
    void {{component.init_func}}({{component.name}} *);
{% endif %}

{% if component.activity%}
    void {{component.activity}}({{component.name}} *);
{% endif %}


#endif
