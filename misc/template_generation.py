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
from SCons.Action import Action

# Helper: prints error message with appropriate prefix.
def print_error(message):
    print "Error: %s\n" % message

def format_title(title, target, source, source_base_dir = ""):
    """
    Format title according to rules below. Returns result of formatting.

    1. Each '%source%' keyword in pattern is replaced with path to the
        first of source file. That path is relative to one given by
        'source_base_dir' variable. If the variable is empty, absolute path is used.
    """
    if title.find('%source%'):
        source_path = source[0].srcnode().abspath
        if source_base_dir != "":
            source_path = os.path.relpath(source_path, source_base_dir)
        title = title.replace('%source%', source_path)

    return title

# Ready made titles.
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
generate_title_python_no_track = '# GENERATED! DO NOT MODIFY!\n'

class CopyWithTitleAction:
    """
    Copy source file into target with appended title.
    Intended to be used as 'action' parameter of SCons '.Command'.
    """
    __slots__ = [
        'title',
        'source_base_dir',
    ]

    def __init__(self, title, source_base_dir = ""):
        """
        'title' - title for created file.
            The title is formatted according to format_title() description.

        'source_base_dir' - if non-empty, path to the source file
            rendered into the title is relative to this directory.
        """
        self.title = title
        self.source_base_dir = source_base_dir

    def __call__(self, target, source, env):
        title = format_title(self.title, target, source, self.source_base_dir)

        try:
            with open(target[0].abspath, 'w') as dest_file:
                dest_file.write(title)
                with open(source[0].path, 'r') as src_file:
                    shutil.copyfileobj(src_file, dest_file)
        except Exception as e:
            print_error("Failed to copy file: %s" % e.message)


def CopyWithTitle(env, target, source, title, source_base_dir = None):
    """
    Copy source file into target with appended title.

    Title is formatted according to format_title() description.
    By default, 'source_base_dir' is env['JETOS_HOME'].

    Use env.AddMethod() for add this builder it into the environment.
    """
    if source_base_dir is None:
        source_base_dir = env['JETOS_HOME']

    t = env.Command(target, source,
        CopyWithTitleAction(title, source_base_dir))

    return t

class ParseContext:
    def __init__(self, filename):
        self.filename = filename
        self.lineno = 0

    def next_line(self):
        self.lineno += 1

    def parse_error(self, message):
        print_error("%s, line %d: %s" % (self.filename, self.lineno, message))

jinja_env_global = jinja2.Environment(
    trim_blocks=True,
    lstrip_blocks=True, # This requires jinja2 >= 2.7
    keep_trailing_newline=True # This requires jinja2 >= 2.7
)


class TemplateRenderAction:
    """
    Copy source file into target with appended title.
    Intended to be used as 'action' parameter of SCons '.Command'.
    """
    __slots__ = [
        'template_file',
        'create_context',
        'title',
        'source_base_dir',
        'jinja_env',
    ]

    def __init__(self, template_file, create_context,
        title = None, source_base_dir = "", **kargs):
        """
        'template_file' is a template file for render.

        'create_context' function should accept 'source' and 'env'
            parameters and return context (as dictionary-like object),
            which will be rendered.

        Optional arguments:

        'title' - if given, it is used as title for prepend rendering content.
            The title is formatted according to format_title() description.

        'source_base_dir' - if non-empty, path to the source file
            rendered into the title is relative to this directory.

        Other arguments are used for redefine parameters of jinja2
        environment 'jinja_env_global' (defined above).
        See description of TemplateRender below for example of such parameters.
        """

        self.template_file = template_file
        self.create_context = create_context
        self.title = title
        self.source_base_dir = source_base_dir

        if kargs:
            self.jinja_env = jinja_env_global.overlay(**kargs)
        else:
            self.jinja_env = jinja_env_global

    def __call__(self, target, source, env):
        context = self.create_context(source, env)

        try:
            with file(self.template_file) as f:
                template_str = f.read().decode('utf-8')
        except IOError as e:
            print "Error while read template file: " + e.message
            return 1

        try:
            template = jinja_env_global.from_string(template_str)
            stream = template.stream(context)

            with open(target[0].path, "w") as f:
                if self.title is not None:
                    f.write(format_title(self.title, target, source, self.source_base_dir))
                stream.dump(f)
        except jinja2.TemplateError as e:
            print "Error while rendering templates: " + e.message
            return 1
        except RuntimeError as e:
            print "Error while interpret template data: " + e.message
            return 1


def TemplateRender(env, target, source, template_file, create_context,
    title = None, source_base_dir = None, **kargs):
    """
    Pseudo builder: Generate 'target' file using jinja2 template rendering.

    'template_file' is a template file for render.

    'create_context' function should accept 'source' and 'env' parameters
        and return context (as dictionary-like object), which will be rendered.

    Optional arguments:

    'title' - if given, it is used as title for prepend rendering content.
        The title is formatted according to format_title() description.

    'source_base_dir' - if non-empty, path to the source file
        rendered into the title is relative to this directory.
        Default is env['JETOS_HOME'].

    Other arguments are used for redefine parameters of jinja2 environment
      'jinja_env_global' (defined above).
    E.g. one may redefine following jinja2 environment parameters
    (default values are given after equal sign):

    - block_start_string = '{%'
        The string marking the beginning of a block.
    - lock_end_string = '%}'
        The string marking the end of a block.
    - variable_start_string = '{{'
        The string marking the beginning of a print statement.
    - variable_end_string = '}}'
        The string marking the end of a print statement.
    - comment_start_string = '{#'
        The string marking the beginning of a comment.
    - comment_end_string = '#}'
        The string marking the end of a comment.
    - line_statement_prefix
        If given and a string, this will be used as prefix for line based statements.
    - line_comment_prefix
        If given and a string, this will be used as prefix for line based comments.

    Use env.AddMethod() for add this builder it into the environment.
    """
    if source_base_dir is None:
        source_base_dir = env['JETOS_HOME']

    template_render_action = TemplateRenderAction(template_file,
        create_context, title, source_base_dir, **kargs)

    t = env.Command(target=target, source=source,
        action=Action(template_render_action, '$JINJACOMSTR'),
    )

    env.Depends(t, template_file)

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


class SyscallBuildAction:
    """
    Build source file from another by expanding SYSCALL_DECLARE definitions.
    Expansion is performed using jinja2 templates.

    Callback for Command(action).
    """
    def __init__(self, template_file, title = None, source_base_dir = ""):
        """
        'template_file' is a template file for render syscalls declarations.

        Optional arguments:

        'title' - if given, it is used as title for prepend rendering content.
            The title is formatted according to format_title() description.

        'source_base_dir' - if non-empty, path to the source file
            rendered into the title is relative to this directory.
        """

        self.template_file = template_file
        self.title = title
        self.source_base_dir = source_base_dir

    def __call__(self, target, source, env):
        jinja_env = jinja_env_global

        try:
            with file(self.template_file) as f:
                template_str = f.read().decode('utf-8')
        except IOError as e:
            print "Error while read template file: " + e.message
            return 1

        try:
            template = jinja_env_global.from_string(template_str)
        except jinja2.TemplateError as e:
            print_error("Failed to load template: %s\n" % e.message)
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

        if self.title is not None:
            output_f.write(format_title(self.title, target, source, self.source_base_dir))

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
                return 1

            if len(args_tokens) % 2 == 1:
                pc.print_syscall_parse_error("Missed argument name after last type")
                return 1

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
            print_error("Unterminated syscall definition")
            return 1

def BuildSyscallDefinition(env, target, source, template_file,
    title = None, source_base_dir = None):
    """
    Pseudo builder: Build 'target' from 'source' file by expanding
    SYSCALL_DECLARE definitions. Expansion is performed using jinja2 templates.

    'template_file' is a template file for render syscall definitions.

    Optional arguments:

    'title' - if given, it is used as title for prepend target's file context.
        The title is formatted according to format_title() description.

    'source_base_dir' - if non-empty, path to the source file
        rendered into the title is relative to this directory.
        Default is env['JETOS_HOME'].

    Use env.AddMethod() for add this builder into the environment.
    """
    if source_base_dir is None:
        source_base_dir = env['JETOS_HOME']

    syscall_build_action = SyscallBuildAction(template_file, title,
        source_base_dir)

    t = env.Command(target, source, syscall_build_action)

    env.Depends(t, template_file)

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

def asm_offsets_build_c_action(target, source, env):
    """
    Build C-source file from C-like file by grouping DEFINE and DEFINE-like
    calls into C-function call.

    Callback for Command(action).
    """
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

class _BuildAsmOffsetsAsmAction:
    """
    Build asm file from C file produced at previous stage.

    Callback for Command(action).
    """
    def __init__(self, title = None, real_source = None, source_base_dir = ""):
        """
        Optional arguments:

        'title' - if given, it is used as title for prepend rendering content.
            The title is formatted according to format_title() description.

        'real_source' - used as 'source' for rendering the title.
            This is because direct source file is actually an intermediate one.

        'source_base_dir' - if non-empty, path to the source file
            rendered into the title is relative to this directory.
        """
        self.title = title
        self.real_source = real_source
        self.source_base_dir = source_base_dir

    def __call__(self, target, source, env):
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

        if self.title is not None:
            output_f.write(format_title(self.title, target, self.real_source, self.source_base_dir))

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


def BuildAsmOffsets(env, target, source, title = None, source_base_dir = None, **kargs):
    """
    Pseudo builder: Build 'target' file from source by expanding
    DEFINE and DEFINE-like calls with C-values into asm definitions `#define`.

    Optional arguments:

    'title' - if given, it is used as title for prepend target's file context.
        The title is formatted according to format_title() description.

    'source_base_dir' - if non-empty, path to the source file
        rendered into the title is relative to this directory.
        Default is env['JETOS_HOME'].

    All other dictionary arguments are assigned to the environment for
    compilation stage when interpret C-values.

    Use AddMethod for add it into the environment.
    """
    if source_base_dir is None:
        source_base_dir = env['JETOS_HOME']

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
        os.path.join(precompile_env['JETOS_HOME'], "misc/asm_offsets")
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

    asm_offsets_build_asm_action = _BuildAsmOffsetsAsmAction(title,
        [source_node], source_base_dir)

    t = env.Command(target, asm_file, asm_offsets_build_asm_action)

    return t
