#!/usr/bin/python

""" Generate header which implements syscall mapping """

import sys
import re
import os

# Maximum number of arguments for system call
SYSCALL_MAX_ARG_NUMBER = 5

class SyscallArg:
    """ Argument for system call """
    def __init__(self, arg_type, arg_name, is_pointer):
        self.arg_type = arg_type
        self.name = arg_name
        self.is_pointer = is_pointer

class SyscallDeclaration:
    """ Declaration of single system call """
    def __init__(self, syscall_id, syscall_func):
        self.syscall_id = syscall_id
        self.func = syscall_func
        self.args = []

# Output given syscall declaration into the kernel's header.
def kernel_output(of, sd):
    of.write("pok_ret_t " + sd.func + "(")
    
    arg_index = 0
    for arg in sd.args:
        arg_index += 1
        if arg_index != 1:
            of.write(",\n    ")
            
        of.write(arg.arg_type + " ")
        # TODO: Uncomment for new scheduler
        #if arg.is_pointer:
        #    of.write("__user ")
        of.write(arg.name)
    
    of.write(");\n")
    
    of.write("static inline pok_ret_t pok_syscall_wrapper_" + sd.syscall_id)
    of.write("(const pok_syscall_args_t* args, const pok_syscall_info_t* infos)\n{\n")

    arg_index = 0
    for arg in sd.args:
        arg_index += 1
        if not arg.is_pointer:
            continue
        of.write("    POK_CHECK_PTR_OR_RETURN(infos->partition, args->arg" + str(arg_index) + " + infos->base_addr)\n")
    
    of.write("    return " + sd.func + "(");
    
    arg_index = 0
    for arg in sd.args:
        arg_index += 1
        if arg_index != 1:
            of.write(",")
        of.write("\n        ");
        # TODO: Uncomment for new scheduler
        #if arg.is_pointer:
            #of.write("(" + arg.arg_type + " __user)")
        #else:
        of.write("(" + arg.arg_type + ")")
        # TODO: For new scheduler this branch is not needed.
        if arg.is_pointer:
            of.write("(args->arg" + str(arg_index) + " + infos->base_addr)")
        else:
            of.write("args->arg" + str(arg_index))
    
    of.write(");\n}\n")
            
        

# Output given syscall declaration into the user's header.
def user_output(of, sd):
    of.write("static inline pok_ret_t " + sd.func + "(")
    
    arg_index = 0
    for arg in sd.args:
        arg_index += 1
        if arg_index != 1:
            of.write(",\n    ")
            
        of.write(arg.arg_type + " " + arg.name)
    
    of.write(")\n{\n")
    
    of.write("    return pok_syscall" + str(len(sd.args)) + "(" + sd.syscall_id)

    for arg in sd.args:
        of.write(",\n        (uint32_t)" + arg.name)
    
    of.write(");\n}\n")
    
    # Selfcheck: remove syscall id definition after we assign function for it.
    of.write("// Syscall should be accessed only by function\n")
    of.write("#undef " + sd.syscall_id + "\n")


def generate_syscalls_declaration_kernel(target, source, env):
    if type(source) is list:
        source = source[0]
    if type(target) is list:
        target = target[0]
    generate_syscalls_declaration(source, target, kernel_output)

def generate_syscalls_declaration_user(target, source, env):
    if type(source) is list:
        source = source[0]
    if type(target) is list:
        target = target[0]
    generate_syscalls_declaration(source, target, user_output)


def generate_syscalls_declaration(infile, outfile, output_func):
    input_f = open(infile.path, "r") or die("Cannot open input file for read")
    output_f = open(outfile.path, "w") or die("Cannot open output file for write")

    # Terminate with error.
    def fatal(msg):
        print msg

    syscall_start_re = re.compile("^SYSCALL_DECLARE\\(")
    syscall_end_re = re.compile("[)]")
    syscall_delim_re = re.compile("\s*,\s*")
    syscall_token_spaces = re.compile("\s+")

    syscall_string = None

    line_count = 0
    syscall_line = 0

    # Terminate because of parse error.
    # Output information about syscall which is currently parsed.
    def fatal_parse_syscall(msg):
        print "Error while parsing syscall at " + str(syscall_line) + ":"
        fatal(msg)

    # Read input file line by line and produce output.
    for line in input_f:
        line_count += 1
        if syscall_string is None:
            if line.startswith("//!"):
                continue
            if not syscall_start_re.match(line):
                output_f.write(line)
                continue
            syscall_string = syscall_start_re.sub("", line)
            syscall_line = line_count
        else:
            syscall_string += line

        if not syscall_end_re.search(syscall_string):
            continue

        syscall_string = syscall_end_re.sub("", syscall_string)
        
        # Original tokens, "as is"
        tokens_orig = syscall_delim_re.split(syscall_string)
        # Tokens with minimal spaces.
        tokens = [syscall_token_spaces.sub(" ", t.strip()) for t in tokens_orig]
        
        if len(tokens) < 2:
            fatal_parse_syscall("Too few tokens for syscall")
        
        sd = SyscallDeclaration(tokens[0], tokens[1])
        
        args_tokens = tokens[2:]
        
        if len(args_tokens) > SYSCALL_MAX_ARG_NUMBER * 2:
            fatal_parse_syscall("Too many arguments for system call. Should be at most " + str(MAX_ARG_NUMBER))
        
        if len(args_tokens) % 2 == 1:
            fatal_parse_syscall("Missed argument name after last type")
        
        for pair in zip(*[iter(args_tokens)]*2):
            is_pointer = 0
            if pair[0].find("*") != -1:
                is_pointer = 1
            sd.args.append(SyscallArg(pair[0], pair[1], is_pointer))

        output_func(output_f, sd)
        
        syscall_string = None

    if syscall_string is not None:
        fatal("Unterminated syscall definition")


def main():
    if len(sys.argv) < 4:
        print "Usage: " + argv[0] + " <input> <output> <output_type>"
        exit(1)

    infile = sys.argv[1]
    outfile = sys.argv[2]
    output_type = sys.argv[3]

    if output_type == "kernel":
        output_func = kernel_output
    elif output_type == "user":
        output_func = user_output
    else:
        print "Incorrect value of <output_type>: " + output_type
        print "Should be either 'kernel' or 'user'"
        exit(1)

    return generate_syscalls_declaration(infile, outfile, output_func)

if __name__ == "__main__":
    main()
