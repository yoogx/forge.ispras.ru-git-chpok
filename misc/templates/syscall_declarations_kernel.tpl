{# TODO: Copyright there -#}
{% macro full_type(arg) %}{{arg.arg_type}}{%if arg.is_pointer%} __user{%endif%}{%endmacro%}
jet_ret_t {{sd.func}}(
{%- for arg in sd.args %}
{{full_type(arg)}} {{arg.name}}{%if not loop.last %},
    {%+endif%}
{%- else%}void
{%- endfor%}
);
static inline jet_ret_t pok_syscall_wrapper_{{sd.syscall_id}}(const pok_syscall_args_t* args)
{
{% if not sd.args %}
    (void) args;
{% endif%}
    return {{sd.func}}({% for arg in sd.args %}{{''}}
        ({{full_type(arg)}})args->arg{{loop.index}}{%if not loop.last %},{%+endif%}
                       {%- endfor%}
);
}
