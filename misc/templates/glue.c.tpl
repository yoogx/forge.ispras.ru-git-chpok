{% for comp in components%}
    #include <{{comp.h_file}}>
    {% for i in comp.instances %}
        void __{{comp.type}}_init__({{comp.type}}*);
        {{comp.type}} {{i.name}} = { .state = {{i.data}} };
    {% endfor %}

{% endfor %}


//#include <Y/y_gen.h>
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
