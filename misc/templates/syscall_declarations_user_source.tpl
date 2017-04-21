{# TODO: Copyright there #}
{% macro full_type(arg) %}{{arg.arg_type}}{%endmacro%}
jet_ret_t _{{sd.func}}_impl(
{%- for arg in sd.args %}
{{full_type(arg)}} {{arg.name}}{%if not loop.last %},
    {%+endif%}
{%- else%}void
{%- endfor%}
)
{
    return pok_syscall{{sd.args | length}}({{sd.syscall_id}}
{%- for arg in sd.args %},
        (uint32_t){{arg.name}}
{%- endfor%});
}

jet_ret_t {{sd.func}}(
{%- for arg in sd.args %}
{{full_type(arg)}} {{arg.name}}{%if not loop.last %},
    {%+endif%}
{%- else%}void
{%- endfor%}
)
__attribute__ ((weak, alias ("_{{sd.func}}_impl")));
