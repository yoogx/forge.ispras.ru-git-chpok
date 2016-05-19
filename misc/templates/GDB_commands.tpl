define Push_to_breakpoints_reason
    set breakpoints[Head_of_breakpoints - 1].Reason = $arg0
    end

document Push_to_breakpoints_reason
Change Reason of last breakpoint
Syntax: push_to_breakpoints_reason REASON
    end
    
define Log_on
    set logging on
    set logging redirect off
    set trace-commands on
    end

document Log_on
Save all information from console (input and output)
    end
    
define Log_off
    set logging off
    end    

define Thread_pid
    set $i = 0
    set $V = 0
    while $i < pok_config_nb_partitions
        if pok_partitions[$i].thread_index_high > $arg0 - 1
            set $V = $i + 1
            loop_break
            end
        set $i = $i + 1
        end
    set $PID = $V
    end

document Thread_pid
Return thread PID
Syntax: thread_pid THREAD_NUM
    end

define Thread_watch
    dont-repeat
    if ($arg1 > pok_config_nb_threads) || ($arg1 < 1)
        printf "Error, there is no such thread\n"
        loop_break
    else
        if (watchpoint_is_set == 0)
            eval "Thread_pid %d",$arg1
            set $V = $PID
            eval "Switch_to %d",$V
            eval "Easy_t_switch %d",$arg1
            set $addr = (uintptr_t) &($arg0)
            set last_breakpoint = last_breakpoint + 1
            set breakpoints[last_breakpoint].Reason = 999
            set Head_of_breakpoints = Head_of_breakpoints + 1
            watch $arg0
            set $V = current_thread + 1
            eval "Thread_pid %d",$V
            set $V = $PID
            eval "Switch_to %d",$V
            eval "Easy_t_switch %d",current_thread + 1
        else
            printf "ERROR, Too many hardware watchpoints!\n"
            end
        end
    end

document Thread_watch
Set watchpoint on chosen thread
Syntax: thread_watch ADDR THREAD_NUMBER
    end

define Thread_delete_w
    dont-repeat
    if (breakpoints[$arg0].B_num != 0) || (breakpoints[$arg0].Reason != 999)
        printf "Error, there is no such watchpoint\n"
        loop_break
    else
        delete $arg0
        end
    end

document Thread_delete_w
Delete watchpoint
Syntax: thread_delete_w BREAKPOINT_NUMBER
    end


define Easy_break
    set $old_Head = Head_of_breakpoints
    Thread_pid $arg1
    break $arg0 thread $arg1
    if ($old_Head == Head_of_breakpoints)
        printf "fix it plz for wrong breakpoint's addr\n"
        set breakpoints[Head_of_breakpoints].addr = (uintptr_t) &($arg0)
        set $Addr = (uintptr_t) breakpoints[Head_of_breakpoints].addr
        set last_breakpoint = last_breakpoint + 1
        set breakpoints[Head_of_breakpoints].T_num = $arg1
        set breakpoints[Head_of_breakpoints].P_num = $PID
        set breakpoints[Head_of_breakpoints].B_num = last_breakpoint
        set $Thread_break_i = 0
        while ((breakpoints[$Thread_break_i].C_addr != $Addr) || (breakpoints[$Thread_break_i].P_num != $PID)) && ($Thread_break_i < max_breakpoint)
            set $Thread_break_i = $Thread_break_i + 1
            end
        if $Thread_break_i != max_breakpoint
            set breakpoints[Head_of_breakpoints].Instr = breakpoints[$Thread_break_i].Instr
            set breakpoints[Head_of_breakpoints].addr = breakpoints[$Thread_break_i].addr        
            set breakpoints[Head_of_breakpoints].C_addr = breakpoints[$Thread_break_i].C_addr        
            set Head_of_breakpoints = Head_of_breakpoints + 1
        else
            printf "Can't find such breakpoint!\n"
            end
        end
    if (breakpoints[Head_of_breakpoints - 1].addr - (uintptr_t) &($arg0) > 0) && (breakpoints[Head_of_breakpoints - 1].addr - (uintptr_t) &($arg0) < 128)
        set breakpoints[Head_of_breakpoints - 1].C_addr = (uintptr_t) &($arg0)
    else
        set breakpoints[Head_of_breakpoints - 1].C_addr = breakpoints[Head_of_breakpoints - 1].addr
        end
    end

document Easy_break
Just add new breakpoint on addr
Syntax: easy_break ADDR THREAD_NUM
    end


define Easy_delete 
    eval "set b_need_to_delete = %d",breakpoints[$arg0].B_num
    eval "delete %d",$arg0
    if (breakpoints[$arg0].T_num > 0)
        set breakpoints[$arg0].T_num = 0
        set breakpoints[$arg0].P_num = 0
        set breakpoints[$arg0].B_num = 0
        set breakpoints[$arg0].addr = 0
        set breakpoints[$arg0].Reason = 0
        end
    set breakpoints[$arg0].C_addr = 0
    end

document Easy_delete
Just delete breakpoint
Syntax: easy_delete BREAKPOINT_NUM
    end

define Easy_t_switch
    if $OLD_THREAD != $arg0
        printf "SWITCH!"
        printf "OLD_THREAD = %d, NEW_THREAD = %d \n", $OLD_THREAD, $arg0
        eval "thread %d", $arg0
        set $OLD_THREAD = $arg0
        end
    end

document Easy_t_switch
Switch to thread
Syntax: easy_t_switch THREAD_NUM
    end

define Easy_switch
    set $G = $arg0 + 1
    if $OLD_PID != $arg0
        printf "SWITCH!!!\n"
        printf "OLD_PID = %d; NEW_PID = %d\n", $OLD_PID, $arg0
        #log_on
        if $OLD_PID != -1
            eval "remove-symbol-file -a 0x%x", pok_partitions[$OLD_PID - 1].base_vaddr
            end
        eval "add-symbol-file pr%d/pr%d.elf 0x%x", $G,$G,pok_partitions[$arg0].base_vaddr
        #log_off
        end
    end

document Easy_switch
Switch to PID by adding a symbol file
Syntax: easy_switch PID
    end

define Switch_to 
    if $arg0 > 0
        set $K = $arg0 - 1
        eval "Easy_switch %d",$K
        set $OLD_PID = $K
        end
    end

document Switch_to
Switch to the partition (load symbol-file if it needs)
Syntax: switch_to PID 
    end


    
    
define Thread_break
    dont-repeat
    if ($arg1 > pok_config_nb_threads) || ($arg1 < 1)
        printf "Error, there is no such thread\n"
        loop_break
    else
        eval "Thread_pid %d",$arg1
        set $V = $PID
        eval "Easy_t_switch %d",$arg1
        eval "Switch_to %d",$V
        eval "set b_need_to_set = 0"
        Easy_break $arg0 $arg1
        eval "Push_to_breakpoints_reason 2"
        set $V = current_thread + 1
        eval "Thread_pid %d",$V
        set $V = $PID
        eval "Switch_to %d",$V
        eval "Easy_t_switch %d",current_thread + 1
        end
    end

document Thread_break
Set breakpoint on chosen thread
Syntax: thread_break ADDR THREAD_NUMBER
    end

define Thread_delete
    dont-repeat
    if ($arg0 < max_breakpoint) && (breakpoints[$arg0].T_num > 0)
        eval "Easy_t_switch %d",breakpoints[$arg0].T_num
        eval "Switch_to %d",breakpoints[$arg0].P_num
        eval "Easy_delete %d", $arg0
        eval "Thread_pid %d",current_thread + 1
        eval "Switch_to %d", $PID
        eval "Easy_t_switch %d",current_thread + 1
    else
        printf "There is no such breakpoint!\n"
        end
    end

document Thread_delete
Delete breakpoint
Syntax: thread_delete BREAKPOINT_NUMBER
    end

define Clear_breakpoints
    set Head_of_breakpoints = 1
    set last_breakpoint = 0
    set new_start = 1
    end
    
document Clear_breakpoints
Clear breakpoints array and set head_of_breakpoint to zero
Syntax: clear_breakpoints
end


define connect_to 
    dont-repeat
    set breakpoint always-inserted on
    set $OLD_PID = -1
    set $OLD_THREAD = -1
    set $P_break_on = -1
    set remotetimeout unlimited
    target extended-remote $arg0
    symbol-file pok.elf
    Clear_breakpoints
    set height 0
    end

document connect_to
Use a remote computer via a serial line, using a gdb-specific protocol.
Specify the serial device it is connected to (e.g. /dev/ttyS0, /dev/ttya, COM1, etc.).
Also clear breakpoints array.
Use 'set breakpoint always-inserted on' option.
Syntax: connect_to PORT_ADDR
    end



define P_break
    dont-repeat
    set $Part_break_i = 0
    set $Max_part = pok_config_nb_partitions
    if $arg0 > $Max_part
        print "Error, there is no such partition"
        loop_break
    else
        if $arg0 == 0
            set $Part_break_i = pok_partitions[$Max_part - 1].thread_index_high        
            Switch_to $arg0
            while $Part_break_i < pok_config_nb_threads
                eval "Easy_t_switch %d",$Part_break_i + 1
                set $addr = $pc
                if $addr != 0x0
                    if $Part_break_i == pok_config_nb_threads - 3 
                        eval "set b_need_to_set = 0"
                        eval "Easy_break monitor %d", $Part_break_i + 1
                        eval "Push_to_breakpoints_reason 1"
                    else
                        if $Part_break_i == pok_config_nb_threads - 4
                            eval "set b_need_to_set = 0"
                            eval "Easy_break gdb %d", $Part_break_i + 1
                            eval "Push_to_breakpoints_reason 1"
                        else
                            eval "set b_need_to_set = 0"
                            eval "Easy_break *0x%x %d",$addr, $Part_break_i + 1
                            eval "Push_to_breakpoints_reason 1"
                            end
                        end
                    end
                set $Part_break_i = $Part_break_i + 1
                end
            eval "Thread_pid %d",current_thread + 1
            eval "Easy_t_switch %d",current_thread + 1
            eval "Switch_to %d",$PID
            loop_break
        else
            set $Part_break_i = pok_partitions[$arg0 - 1].thread_index_low        
            Switch_to $arg0
            while $Part_break_i < pok_partitions[$arg0 - 1].thread_index_high
                eval "Easy_t_switch %d",$Part_break_i + 1
                set $addr = $pc
                if $addr != 0x0
                    eval "set $V = *((int *) %d)",$pc
                    eval "set b_need_to_set = 0"
                    eval "Easy_break *0x%x %d",$addr, $Part_break_i + 1
                    eval "Push_to_breakpoints_reason 1"
                    end
                set $Part_break_i = $Part_break_i + 1
                end
            eval "Thread_pid %d",current_thread + 1
            eval "Easy_t_switch %d",current_thread + 1
            eval "Switch_to %d",$PID
            end
        end
    set $P_break_on = $arg0
    end
    

document P_break
Set breakpoints on all partition's threads
Delete all breakpoints after next break
Syntax: p_break PID
    end
    
define P_delete
    dont-repeat
    set $Max_part = pok_config_nb_partitions
    if $arg0 > $Max_part
        printf "Error, there is no such partition\n"
        loop_break
    else
        if Head_of_breakpoints > 0
            if $arg0 > 0
                eval "Easy_t_switch %d",pok_partitions[$arg0 - 1].thread_index_low + 1
                end
            Switch_to $arg0
            set $P_delete_i = Head_of_breakpoints - 1
            while ((breakpoints[$P_delete_i].P_num != $arg0) || (breakpoints[$P_delete_i].Reason != 1)) && ($P_delete_i + 1 > 0)
                set $P_delete_i = $P_delete_i - 1
                end
            if $P_delete_i + 1 > 0
                while (breakpoints[$P_delete_i].P_num == $arg0) && ($P_delete_i + 1 > 0) && (breakpoints[$P_delete_i].Reason == 1)
                    eval "Easy_t_switch %d",breakpoints[$P_delete_i].T_num
                    eval "Easy_delete %d",  $P_delete_i
                    set $P_delete_i = $P_delete_i - 1
                    end
                eval "Thread_pid %d",current_thread + 1
                eval "Easy_t_switch %d",current_thread + 1
                eval "Switch_to %d",$PID           
            else
                printf "ERROR, there was't p_break for this partition\n"
                end
            end
        end
    end
    
document P_delete
Delete breakpoints were set by P_break. Use it only after break,before adding any breakpoints
Syntax: p_delete PID 
    end    
    
define P_info
    dont-repeat
    set verbose off
    set $P_info_old_pid = -1
    if $argc == 0
        set $P_info_i = pok_config_nb_threads
        while $P_info_i > 0
            eval "Thread_pid %d",$P_info_i
            if $P_info_old_pid != $PID
                eval "Switch_to %d",$PID
                end
            eval "info thread %d",$P_info_i            
            set $P_info_i = $P_info_i - 1
            set $P_info_old_pid = $PID
            end
        eval "Thread_pid %d",current_thread + 1
        if $P_info_old_pid != $PID
            eval "Switch_to %d",$PID
            end
    else
        eval "Thread_pid %d",$arg0
        eval "Switch_to %d",$PID
        eval "info thread %d",$arg0            
        eval "Thread_pid %d",current_thread + 1
        eval "Switch_to %d",$PID
        end
    end
    
document P_info
Print information about all or chosen threads in system. Similar to "info thread", but it loads symbol files.
Syntax: p_info THREAD_NUMBER
    end    

define P_mem
    set $Max_part = pok_config_nb_threads
    if ($arg0 > $Max_part) || ($arg0 < 1)
        printf "Error, there is no such thread\n"
        loop_break
    else
        eval "Thread_pid %d",$arg0
        eval "Easy_t_switch %d",$arg0
        eval "Switch_to %d",$PID
        if $argc == 2
            $arg1
            end
        if $argc == 3
            $arg1 $arg2
            end
        if $argc == 4
            $arg1 $arg2 $arg3
            end
        if $argc == 5
            $arg1 $arg2 $arg3 $arg4
            end
        eval "Thread_pid %d",current_thread + 1
        eval "Easy_t_switch %d",current_thread + 1
        eval "Switch_to %d",$PID
        end
    end    

document P_mem
All simple operations. (read/write in memory, info registers, etc.)
Syntax: p_mem THREAD_NUM OPERATIONS ARG1 ARG2 ...
    end
    
define test
    connect_to  127.0.0.1:8002
    add-inferior -copies 1 -exec pr1/pr1.elf
    inferior 2
    #add-symbol-file pr1/pr1.elf 0x80000000
    attach 2
    inferior 1
    test2
    end

define test2
    add-inferior -copies 1 -exec pr2/pr2.elf
    inferior 3
    #add-symbol-file pr2/pr2.elf 0x80000000
    attach 3
    inferior 1
    end

#define add_new_inferior
#    eval "add-inferior -copies 1 -exec P%d/build/e500mc/part.elf", $arg0
#    eval "inferior %d", $arg0 + 1
#    eval "attach %d", $arg0 + 1
#    end

define target hookpost-extended-remote
    set breakpoint always-inserted on
    set remotetimeout unlimited
    set schedule-multiple on
    set height 0
{% for part_build_dir in partition_build_dirs %}
    add-inferior -copies 1 -exec {{part_build_dir}}/part.elf"
    inferior {{ loop.index0 + 2 }}
    attach {{ loop.index0 + 2 }}
    
    #set $GDB_part = pok_config_nb_partitions
    #set $GDB_i = 1
    #while ($GDB_i <= $GDB_part)
    #    eval "add_new_inferior %d", $GDB_i
    #    set $GDB_i = $GDB_i + 1
    #    end
{% endfor %}
    inferior 1
    end
