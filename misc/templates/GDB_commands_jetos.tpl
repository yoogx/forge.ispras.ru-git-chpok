define target hookpost-extended-remote
    set breakpoint always-inserted on
    set remotetimeout unlimited
    set schedule-multiple on
    set height 0
{% for part in conf.partitions %}
    add-inferior -copies 1 -exec {{partitions_elf_map[part.name]}}"
    inferior {{ loop.index0 + 2 }}
    attach {{ loop.index0 + 2 }}
{% endfor %}
    inferior 1
    end

define hook-continue
    eval "thread %d", $Current_thread
    end
    
define hook-stop
    py gdb.execute('set $Current_thread = '  + gdb.execute('thread', to_string=True).split(' ')[3])
    end

target extended-remote 127.0.0.1:8000
