#ifndef __INTERFACES_{{interface.name|upper}}_H__
#define __INTERFACES_{{interface.name|upper}}_H__

#include <lib/common.h>
{% for add_h in interface.additional_h_files %}
    #include <{{add_h}}>
{% endfor %}

typedef struct {
  {% for func in interface.functions %}
    {{func.return_type}} (*{{func.name}})({{func.args_type|join(', ') }});
    {% endfor %}
} {{interface.name}};


#endif

