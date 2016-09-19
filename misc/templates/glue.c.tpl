{% for comp in components%}
    #include <{{comp.type}}_gen.h>
    {% for i in comp.instances %}
        void __{{comp.type}}_init__({{comp.type}}*);
        void __{{comp.type}}_activity__({{comp.type}}*);
        {{comp.type}} {{i.name}} = {
            {% if i.state %}
            .state = {
            {% for name, val in i.state.iteritems()%}
                .{{name}} = {{val}},
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
        {{l.from.instance}}.out.{{l.from.port}}.ops = &{{l.to.instance}}.in.{{l.to.port}}.ops;
        {{l.from.instance}}.out.{{l.from.port}}.owner = &{{l.to.instance}};
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
