#******************************************************************
#
# Institute for System Programming of the Russian Academy of Sciences
# Copyright (C) 2016 ISPRAS
#
#-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation, Version 3.
#
# This program is distributed in the hope # that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#
# See the GNU General Public License version 3 for more details.
#
#-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=


import sys
import os
import re
import subprocess
import shutil

import jinja2

# Helper: prints error message with appropriate prefix.
def print_error(message):
    print "Error: %s\n" % message

def format_title(title, target, source, env):
    """
    Format title according to rules below. Returns result of formatting.

    1. Each '%source%' keyword in pattern is replaced with path to the
        first of source file. That path is relative to one given by
        'SOURCE_BASE_DIR' variable. If absent, path relative to current
        directory is calculated.
    """
    if title.find('%source%'):
        source_base_dir=env.get('SOURCE_BASE_DIR')
        if source_base_dir is None:
            source_base_dir = env.Dir('.').abspath
        source_path = os.path.relpath(source[0].srcnode().abspath, source_base_dir)
        title = title.replace('%source%', source_path)

    return title

copy_title_c = """/*
 * COPIED! DO NOT MODIFY!
 *
 * Instead of modifying this file, modify original one (%source%).
 */
"""

generate_title_c = """/*
 * GENERATED! DO NOT MODIFY!
 *
 * Instead of modifying this file, modify the one it generated from (%source%).
 */
"""

generate_title_c_no_track = '/* GENERATED! DO NOT MODIFY! */\n'
generate_title_python = '# GENERATED! DO NOT MODIFY!\n'

generate_title_python = """# GENERATED! DO NOT MODIFY!
#
# Instead of modifying this file, modify the one it generated from (%source%).
"""


def CopyWithTitle(target, source, env):
    """
    Copy source file into target with appended title.
    Intended to be used as 'action' parameter of SCons '.Command'.

    Title should be given by 'COPY_TITLE' environment variable. It is
    formatted according to format_title() description.
    """

    title = env.get('COPY_TITLE')
    if title is None:
        print_error("AddTitle: 'COPY_TITLE' variable is not set")
        return 1

    title = format_title(title, target, source, env)

    try:
        with open(target[0].abspath, 'w') as dest_file:
            dest_file.write(title)
            with open(source[0].path, 'r') as src_file:
                shutil.copyfileobj(src_file, dest_file)
    except Exception as e:
        print_error("Failed to copy file: %s" % e.message)


class ParseContext:
    def __init__(self, filename):
        self.filename = filename
        self.lineno = 0

    def next_line(self):
        self.lineno += 1

    def parse_error(self, message):
        print_error("%s, line %d: %s" % (self.filename, self.lineno, message))

class TemplateLoader(jinja2.BaseLoader):
    """ Template loader wich locates templates in the 'path/%name%.tpl'.
    """
    def __init__(self, path, debug=False, used=None):
        self.path = path.rstrip("/")
        self.debug = debug
        self.used = used
    
    def get_source(self, environment, template):
        filename = os.path.join(self.path, template + '.tpl')
        if not os.path.exists(filename):
            if self.debug:
                source = '{TODO:' + template + '}' 
                return source, None, lambda: True
            else:
                raise jinja2.TemplateNotFound(template,
                    message = "Couldn't find template '" + template + "' under '" + self.path + "'")
        mtime = os.path.getmtime(filename)
        with file(filename) as f:
            source = f.read().decode('utf-8')
        # Update list of used templates.
        if self.used is not None:
            self.used.add(filename)
        return source, filename, lambda: mtime == os.path.getmtime(filename)

jinja_env_global = jinja2.Environment(
    trim_blocks=True,
    lstrip_blocks=True, # This requires jinja2 >= 2.7
    keep_trailing_newline=True # This requires jinja2 >= 2.7
)


# Create source file by rendering template.
#
# Callback for Command(action).
#
# Required environment variables:
#
# 'TEMPLATE_DIR' - directory from which templates will be used,
# 'TEMPLATE_MAIN' - main template for render (one per-target)
# 'TEMPLATE_CREATE_DEFINITIONS_FUNC' - function, which accepts our 'source'
#     and 'env' parameters and return definitions, which will be rendered.
#
# Optional environment variables:
#
# 'GENERATE_TITLE' - if set, it is used as title for prepend rendering
#     content. The title is formatted according to format_title() description.
# 'NO_DEPS' - if set, per-target `.dep` file won't be created. This file
#      contains dependency list of all templates used when generate particular file.
#      Without this file only dependency from main template file is tracked.
def template_render_action(target, source, env):
    template_dir = env['TEMPLATE_DIR']
    if template_dir is None:
        raise RuntimeError('TEMPLATE_DIR is not specified for renderer')
    
    template_main = env['TEMPLATE_MAIN']
    if template_main is None:
        raise RuntimeError('TEMPLATE_MAIN is not specified for renderer')
    
    template_create_definitions_func = env['TEMPLATE_CREATE_DEFINITIONS_FUNC']
    if template_create_definitions_func is None:
        raise RuntimeError('TEMPLATE_CREATE_DEFINITIONS_FUNC is not specified for renderer')

    no_deps = env.get('NO_DEPS')
    
    # Create some tree from the source files in memory
    tree = template_create_definitions_func(source, env)
    
    used_list = None
    if not no_deps:
        used_list = set()
    # Create jinja environment with specific template loader.
    loader = TemplateLoader(template_dir, used = used_list)
    jinja_env = jinja_env_global.overlay(loader = loader)
    
    for target_single, template_main_single in zip(target, template_main):
        try:
            # Choose template for instantiate
            template = jinja_env.get_template(template_main_single)

            # Render template into stream...
            stream = template.stream(tree)
            # ..and dump stream itself into file.
            with open(target_single.path, "w") as f:
                generate_title = env.get('GENERATE_TITLE')
                if generate_title is not None:
                    f.write(format_title(generate_title, target, source, env))
                stream.dump(f)

            if not no_deps:
                deps_file_path = target_single.path + '.deps'
                with open(deps_file_path, "w") as deps_file:
                    for f in used_list:
                        deps_file.write("%s: %s\n" % (target_single.abspath, f))
        except jinja2.TemplateError as e:
            print "Error while rendering templates: " + e.message
            return 1

# Pseudo builder(method).
#
# Generate 'target' file(s) using jinja2 template rendering.
#
# Function 'create_definitions_func' should accept 'source' and 'env'
# parameters and return map(dict) of definitions, which will be rendered.
#
# 'template_main' is a list of per-target template names for render.
#
# 'template_dir' is a (single) directory, when templates should be located.
# File '{template_dir}/{template_name}.tpl' correcponds to template
# '{template_name}'.
#
# In case of single target, both 'target' and 'template_main' may be single
# objects, not a list.
#
# All other dictionary arguments are assigned to the environment.
#
# Optional environment variables which affects behaviour:
#
# 'GENERATE_TITLE' - if set, it is used as title for prepend rendering
#     content. The title is formatted according to format_title() description.
# 'NO_DEPS' - if set, per-target `.dep` file won't be created. This file
#      contains dependency list of all templates used when generate particular file.
#      Without this file only dependency from main template file is tracked.
#
# Use env.AddMethod() for add this builder it into the environment.
def TemplateRender(env, target, source, create_definitions_func,
    template_main, template_dir, **kargs):

    if type(target) is not list:
        target = [target]

    if type(template_main) is not list:
        template_main = [template_main]

    if len(target) != len(template_main):
        raise RuntimeError("'target' and 'template_main' lists have different lengths.")

    t = env.Command(target,
                source,
                template_render_action,
                TEMPLATE_DIR = template_dir,
                TEMPLATE_MAIN = template_main,
                TEMPLATE_CREATE_DEFINITIONS_FUNC = create_definitions_func,
                **kargs
                )

    for target_single, template_main_single in zip(t, template_main):
        if env.get('NO_DEPS') or kargs.get('NO_DEPS'):
            main_template_file = os.path.join(template_dir, template_main_single + '.tpl')
            env.Depends(target_single, main_template_file)
        else:
            env.SideEffect(target_single.abspath + '.deps', t)
            env.ParseDepends(target_single.abspath + '.deps')

    return t

######################### Build system calls definitions ###############

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

class ParseError(RuntimeError):
    def __init__(self, args,):
        RuntimeError.__init__(self, args)

class ParseContextSyscallDeclaration(ParseContext):
    def __init__(self, filename):
        ParseContext.__init__(self, filename)
        self.syscall_lineno = None

    def syscall_start(self):
        self.syscall_lineno = self.lineno

    def print_syscall_parse_error(self, message):
        print_error("Failed to parse syscall at %d: %s" % (self.syscall_lineno, message))


# Build source file from another by expanding SYSCALL_DECLARE definitions.
# Expansion is performed using jinja2 templates.
#
# Callback for Command(action).
#
# Required environment variables:
#
# 'TEMPLATE_DIR' - directory from which templates will be used,
# 'TEMPLATE_MAIN' - main template for render
#
# If 'GENERATE_TITLE' variable is set, it is used as title
# for prepend rendering content. The title is formatted according to
# format_title() description.
def syscall_build_action(target, source, env):
    template_dir = env.get('TEMPLATE_DIR')
    if template_dir is None:
        print_error("TEMPLATE_DIR is not specified for template renderer")
        return 1

    template_main = env.get('TEMPLATE_MAIN')
    if template_main is None:
        print_error("TEMPLATE_MAIN is not specified for renderer\n")
        return 1

    # Create jinja environment with specific template loader.
    loader = TemplateLoader(template_dir)

    jinja_env = jinja_env_global.overlay(loader = loader)

    try:
        template = jinja_env.get_template(template_main)
    except jinja2.TemplateError as e:
        print_error("Failed to load main template: %s\n" % e.message)
        return 1
    # Parse input and produce output
    input_path = source[0].path
    input_f = open(input_path, "r")
    if not input_f:
        print_error("Cannot open file %s for read syscalls\n" % input_path)
        return 1
    output_path = target[0].abspath
    output_f = open(output_path, "w")
    if not output_f:
        print_error("Cannot open file %s for write syscalls\n" % output_path)
        return 1

    generate_title = env.get('GENERATE_TITLE')
    if generate_title is not None:
        output_f.write(format_title(generate_title, target, source, env))


    syscall_start_re = re.compile("^SYSCALL_DECLARE\\(")
    syscall_end_re = re.compile("[)]")
    syscall_delim_re = re.compile("\s*,\s*")
    syscall_token_spaces = re.compile("\s+")

    syscall_string = None

    pc = ParseContextSyscallDeclaration(input_path)

    # Read input file line by line and produce output.
    for line in input_f:
        pc.next_line()
        if syscall_string is None:
            if line.startswith("//!"):
                continue
            if not syscall_start_re.match(line):
                output_f.write(line)
                continue
            syscall_string = syscall_start_re.sub("", line)
            pc.syscall_start()
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
            pc.print_syscall_parse_error("Too few tokens for syscall")
            return 1

        sd = SyscallDeclaration(tokens[0], tokens[1])

        args_tokens = tokens[2:]

        if len(args_tokens) > SYSCALL_MAX_ARG_NUMBER * 2:
            pc.print_syscall_parse_error("Too many arguments for system call. Should be at most " + str(MAX_ARG_NUMBER))

        if len(args_tokens) % 2 == 1:
            pc.print_syscall_parse_error("Missed argument name after last type")

        for pair in zip(*[iter(args_tokens)]*2):
            is_pointer = 0
            if pair[0].find("*") != -1:
                is_pointer = 1
            sd.args.append(SyscallArg(pair[0], pair[1], is_pointer))

        try:
            output_f.write(template.render(sd = sd))
        except jinja2.TemplateError as e:
            print_error("Failed to render syscall definition at %s: %s" % (pc.syscall_lineno, e.message))
            return 1

        syscall_string = None

    if syscall_string is not None:
        fatal("Unterminated syscall definition")

# Pseudo builder(method).
#
# Build 'target' from 'source' file by expanding SYSCALL_DECLARE definitions.
# Expansion is performed using jinja2 templates.
#
# 'template_main' is a template for render.
#
# 'template_dir' is a (single) directory, when templates should be located.
# File '{template_dir}/{template_name}.tpl' correcponds to template
# '{template_name}'.
#
# All other dictionary arguments are assigned to the environment.
#
# If 'GENERATE_TITLE' variable is set, it is used as title
# for prepend rendering content. The title is formatted according to
# format_title() description.
#
# Use env.AddMethod() for add this builder into the environment
def BuildSyscallDefinition(env, target, source, template_main,
    template_dir, **kargs):

    if type(target) is list:
        target = target[0]

    t = env.Command(target,
                source,
                syscall_build_action,
                TEMPLATE_DIR = template_dir,
                TEMPLATE_MAIN = template_main,
                **kargs
                )

    env.Depends(t, template_dir + '/' + template_main + '.tpl')

    return t

######################### Build asm file with offsets ###############

class ParseContextAsmOffsets(ParseContext):
    def __init__(self, filename):
        ParseContext.__init__(self, filename)
        self.current_function_name = None
        self.function_next_index = 1

    def start_function(self):
        assert(self.current_function_name is None)
        self.current_function_name = "foo" + str(self.function_next_index)
        self.function_next_index += 1
        return self.current_function_name

    def end_function(self):
        assert(self.current_function_name is not None)
        self.current_function_name = None

    def get_function(self):
        return self.current_function_name


def stringify_comment(comment):
    escaped_comment = re.sub(r'([\\"\'?])', r'\\\1', comment)
    escaped_comment = re.sub('\n', "\\\\n", escaped_comment)
    return "\"" + escaped_comment + "\""

# Build C-source file from C-like file by grouping DEFINE and DEFINE-like
# calls into C-function call.
#
# Callback for Command(action).
#
# If 'GENERATE_TITLE' variable is set, it is used as title
# for prepend rendering content. The title is formatted according to
# format_title() description.
def asm_offsets_build_c_action(target, source, env):
    # Parse input and produce output
    input_path = source[0].path
    input_f = open(input_path, "r")
    if not input_f:
        print_error("Cannot open file %s for read asm definitions\n" % input_path)
        return 1
    output_path = target[0].abspath
    output_f = open(output_path, "w")
    if not output_f:
        print_error("Cannot open file %s for write C file with asm definitions\n" % output_path)
        return 1

    is_multiline_comment = False
    output_f.write("#include <build_asm_offsets.h>\n")

    pc = ParseContextAsmOffsets(input_path)
    # Read input file line by line and produce output.
    for line in input_f:
        pc.next_line()

        if line.startswith("#") and not is_multiline_comment:
            # Directive '#include' goes into C-file without changes.
            # Same for the macro definitions/checks.

            # Make sure that we are not in the function scope.
            if pc.get_function() is not None:
                output_f.write("}\n")
                pc.end_function()

            output_f.write(line)
            continue

        # Everything else should be 'packed' into the resulted asm header.
        # Make sure that we are in the function scope.
        if pc.get_function() is None:
            func_name = pc.start_function()
            output_f.write("void %s(void)\n{\n" % func_name)

        if not is_multiline_comment and re.match("^(DEFINE|OFFSETOF|SIZEOF_STRUCT)", line):
            # DEFINE or DEFINE-like definition
            directive = line.rstrip("\n")
            output_f.write("    %s;\n" % directive)
            continue

        # Everything else 'packed' AS IS.
        # We only need to determine comments border.
        line_pos = 0

        space_only_pattern = re.compile("^\\s*|\n$")
        comment_start_pattern = re.compile("^\\s*(//)|(/\\*)")
        comment_end_pattern = re.compile("^.*?\\*/")

        while not space_only_pattern.match(line, line_pos):
            if not is_multiline_comment:
                comment_match = comment_start_pattern.match(line, line_pos)
                if comment_match:
                    if comment_match.group(1):
                        # Full-line comment
                        break
                    else:
                        # Multiline comment starts
                        is_multiline_comment = True
                        line_pos = comment_match.end()
                else:
                    pc.parse_error("Non-space character outside of comments")
                    return 1
            else:
                # Attempt to match to the first '*/'
                comment_end_match = comment_end_pattern.match(line, line_pos)
                if comment_end_match:
                    # Multiline comment ends
                    line_pos = comment_end_match.end()
                    is_multiline_comment = False
                else:
                    # Multiline comment continues up to end of line
                    break

        output_f.write("    AS_IS(%s);\n" % stringify_comment(line))

    # Finish function's definition if needed.
    if pc.get_function() is not None:
        output_f.write("}\n")
        pc.end_function()

    input_f.close()
    output_f.close()

# Build asm file from C file produced at previous stage.
#
# Callback for Command(action).
#
# If 'GENERATE_TITLE' variable is set, it is used as title
# for prepend rendering content. The title is formatted according to
# format_title() description.
#
# Because direct source file is actually intermediate one, for generate
# title use filename contained in REAL_SOURCE environment variable.
def asm_offsets_build_asm_action(target, source, env):
    # Parse input and produce output
    input_path = source[0].path
    input_f = open(input_path, "r")
    if not input_f:
        print_error("Cannot open file %s for read encoded asm definitions\n" % input_path)
        return 1

    output_path = target[0].abspath
    output_f = open(output_path, "w")
    if not output_f:
        print_error("Cannot open file %s for write asm definitions\n" % output_path)
        return 1

    generate_title = env.get('GENERATE_TITLE')
    if generate_title is not None:
        real_source = env['REAL_SOURCE']
        output_f.write(format_title(generate_title, target, real_source, env))

    # Read input file line by line and produce output.
    for line in input_f:
        define_match = re.match("-> (\\w+) (\\$)?(\\w+)", line)
        if define_match:
            output_f.write("#define %s %s\n" % (
                define_match.group(1),
                define_match.group(3)
            ))
            continue
        comment_match = re.match("->#(.*)", line)
        if comment_match is not None:
            comment = comment_match.group(1)
            output_f.write("%s\n" % comment)
            continue

    input_f.close()
    output_f.close()


# Pseudo builder(method).
#
# Build 'target' from source 'file' by expanding DEFINE and DEFINE-like
# calls with C-values into asm definitions `#define`.
#
# All other dictionary arguments are assigned to the environment.
#
# If 'GENERATE_TITLE' variable is set, it is used as title
# for prepend rendering content. The title is formatted according to
# format_title() description.
#
# Use AddMethod for add it into the environment
def BuildAsmOffsets(env, target, source, **kargs):
    source_node = env.File(source)
    target_node = env.File(target)

    (target_dir,target_name) = os.path.split(target_node.abspath)

    # Even if both source and target files are in source directory,
    # intermediate files should be created in build directory
    build_dir = env.Dir('.').abspath

    c_filename = os.path.join(build_dir, "." + target_name + ".c")
    asm_filename = os.path.join(build_dir, "." + target_name + ".asm")

    # Create new environment for redefine certain variables.
    precompile_env = env.Clone(**kargs)

    # Add '-S' flag for skip compilation stage
    precompile_env.AppendUnique(CCFLAGS = '-S')
    # Include directory with header defined DEFINE and other macros.
    precompile_env.Append(CPPPATH =
        os.path.join(precompile_env['POK_PATH'], "misc/asm_offsets")
    )

    source_dir = os.path.dirname(source_node.srcnode().abspath)

    # Directory with C-file itself is included automatically.
    #
    # But in case when C-file is in build directory, but source one is
    # in source directory, we need to include source directory explicitely
    precompile_env.Append(CPPPATH = source_dir)

    c_file = precompile_env.Command(c_filename,
                source,
                asm_offsets_build_c_action
                )

    asm_file = precompile_env.Object(asm_filename, c_file)

    # Pass original source file for correct title generated.
    precompile_env['REAL_SOURCE'] = [source_node]

    t = precompile_env.Command(target,
                asm_file,
                asm_offsets_build_asm_action
                )

    return t
