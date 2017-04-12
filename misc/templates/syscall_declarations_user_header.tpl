{# TODO: Copyright there #}
{% macro full_type(arg) %}{{arg.arg_type}}{%endmacro%}
pok_ret_t {{sd.func}}(
{%- for arg in sd.args %}
{{full_type(arg)}} {{arg.name}}{%if not loop.last %},
    {%+endif%}
{%- else%}void
{%- endfor%}
);
