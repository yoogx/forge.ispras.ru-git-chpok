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
    block_start_string='<$',
    block_end_string='$>',
    comment_start_string='<#',
    comment_end_string='#>',
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
# 'TEMPLATE_MAIN' - main template for render
# 'TEMPLATE_CREATE_TREE_FUNC' - function, which accepts our 'source'
#     and 'env' parameters and return tree, which will be rendered.
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
    
    template_create_tree_func = env['TEMPLATE_CREATE_TREE_FUNC']
    if template_create_tree_func is None:
        raise RuntimeError('TEMPLATE_CREATE_TREE_FUNC is not specified for renderer')
    template_create_used = env['TEMPLATE_CREATE_USED']
    
    # Create some tree from the source files in memory
    # TODO: It should be a way for generate several targets from single tree
    tree = template_create_tree_func(source, env)
    
    used_list = None
    if template_create_used is not None:
        used_list = set()
    # Create jinja environment with specific template loader.
    loader = TemplateLoader(template_dir, used = used_list)
    jinja_env = jinja_env_global.overlay(loader = loader)
    
    try:
        # Choose template for instantiate
        template = jinja_env.get_template(template_main)
        
        # Render template into stream...
        stream = template.stream(tree)
        # ..and dump stream itself into file.
        if type(target) is list:
            target = target[0]
        
        with open(target.path, "w") as f:
            stream.dump(f)

        if template_create_used is not None:
            deps_file_path = target.path + '.deps'
            with open(deps_file_path, "w") as deps_file:
                for f in used_list:
                    deps_file.write("%s: %s\n" % (target.path, f))
    except jinja2.TemplateError as e:
        print "Error while rendering templates: " + e.message
        return 1

# Pseudo builder(method)
#
# Use AddMethod for add it into the environment
def TemplateRender(env, target, source, create_tree_func,
    template_main, template_dir):
    t = env.Command(target,
                source,
                template_render_action,
                TEMPLATE_DIR = template_dir,
                TEMPLATE_MAIN = template_main,
                TEMPLATE_CREATE_TREE_FUNC = create_tree_func,
                TEMPLATE_CREATE_USED = 1
                )
    
    # TODO: other work
    env.SideEffect(target + '.deps', t)
    env.ParseDepends(target + '.deps')

