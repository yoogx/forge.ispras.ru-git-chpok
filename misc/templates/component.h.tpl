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
 {% if component.state_struct %}
  {%for name, type in component.state_struct.iteritems()%}
    {{type}} {{name}};
  {% endfor %}
 {% endif %}
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
          {% if p.is_array %}
            struct {
                {{p.type}} *ops;
                self_t *owner;
            } *{{p.name}};
          {% else %}
            struct {
                {{p.type}} *ops;
                self_t *owner;
            } {{p.name}};
          {% endif %}
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
    {% if p.is_array %}
      {{i_func.return_type}} {{component.name}}_call_{{p.name}}_{{i_func.name}}_by_index(int, {{component.name}} *{{args(i_func)}});
    {% else %}
      {{i_func.return_type}} {{component.name}}_call_{{p.name}}_{{i_func.name}}({{component.name}} *{{args(i_func)}});
    {% endif %}
  {% endfor %}
{% endfor %}



{% if component.init_func %}
    void {{component.init_func}}({{component.name}} *);
{% endif %}

{% if component.activity%}
    void {{component.activity}}({{component.name}} *);
{% endif %}


#endif
