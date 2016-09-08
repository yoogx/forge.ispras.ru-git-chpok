#ifndef __{{component.name}}_GEN_H__
#define __{{component.name}}_GEN_H__

{% for h in component.additional_h_files %}
    #include <{{h}}>
{% endfor %}

{% for p in component.in_ports %}
    #include <interfaces/{{p.type}}_gen.h>
{% endfor %}

{% for p in component.out_ports %}
    #include <interfaces/{{p.type}}_gen.h>
{% endfor %}

struct {{component.name}}_state {
    {% for field in component.state_struct %}
        {{field.type}} {{field.name}};
    {% endfor %}
};

typedef struct {
    struct {{component.name}}_state state;
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


{% for p in component.in_ports %}
  {% for i_func in interfaces[p.type].functions %}
      {{i_func.return_type}} {{p.implementation[i_func.name]}}({{component.name}} *
      {%for arg in i_func.args_type[1:]%}, {{arg}}{%endfor%});
  {% endfor %}
{% endfor %}



{#
{% if comp == 'x' %}
int x_send(X *self, void *buf, size_t len);
void x_flush(X *self);
{% else %}
void y_tick(Y *self);
{% endif %}
#}

{% if component.init_func %}
    void {{component.init_func}}({{component.name}} *);
{% endif %}

#endif
