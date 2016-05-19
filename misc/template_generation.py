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

import jinja2

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
    trim_blocks=True
# For some reasons, jinja 2.6 lack of support for 'lstrip_block' parameter
# (but support {%- %} in templates )
# jinja 2.6 lack of support for 'keep_trailing_newline' parameter
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
# 'TEMPLATE_CREATE_USED' - if set, during rendering`.dep` file will
#     be created. This file will contain list of templates used.
#     
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

    template_create_used = env['TEMPLATE_CREATE_USED']
    
    # Create some tree from the source files in memory
    tree = template_create_definitions_func(source, env)
    
    used_list = None
    if template_create_used is not None:
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
                stream.dump(f)

            if template_create_used is not None:
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
# Use AddMethod for add it into the environment
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
                TEMPLATE_CREATE_USED = 1,
                **kargs
                )

    for target_single in target:
        env.SideEffect(target_single + '.deps', t)
        env.ParseDepends(target_single + '.deps')

