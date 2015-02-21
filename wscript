#!/usr/bin/env python
import os
import subprocess

import waflib.Logs as Logs, waflib.Options as Options
from waflib.extras import autowaf as autowaf

# Variables for 'waf dist'
VERSION = '0.0.1'
APPNAME = 'drobillad'

# Mandatory variables
top = '.'
out = 'build'

projects = '''
    serd
    sord
    sratom
    suil
    lilv
    raul
    ganv
    patchage
    ingen
    jalv
    machina
    naub
    plugins/float.lv2
    plugins/mda.lv2
    plugins/matriseq
    plugins/blop.lv2
    plugins/fomp.lv2
    plugins/omins.lv2
    plugins/glamp.lv2
    plugins/mesp.lv2
'''.split()
#    plugins/dirg.lv2
#    plugins/lolep.lv2

def options(opt):
    opt.load('compiler_c')
    opt.load('compiler_cxx')
    opt.load('lv2')
    autowaf.set_options(opt)
    opt.add_option('--cmd', type='string', dest='cmd',
                   help='Command to run from build directory (for run command)')
    for i in projects:
        opt.recurse(i)

def sub_config_and_use(conf, name, has_objects = True, pkgname = ''):
    conf.recurse(name)
    if pkgname == '':
        pkgname = name
    autowaf.set_local_lib(conf, pkgname, has_objects)

def configure(conf):
    conf.load('compiler_c')
    conf.load('compiler_cxx')
    conf.load('lv2')
    autowaf.configure(conf)
    autowaf.set_recursive()

    autowaf.check_pkg(conf, 'lv2', atleast_version='1.11.0', uselib_store='LV2')

    # I have no idea why this is necessary
    conf.env.CXXFLAGS += ['-I%s/raul' % os.path.abspath(top)]

    print('')
    conf.env.DROBILLAD_BUILD = []
    global to_build
    for i in projects:
        try:
            sub_config_and_use(conf, i)
            conf.env.DROBILLAD_BUILD += [i]
        except:
            Logs.warn('Configuration failed, %s will not be built\n' % i)

    Logs.info('Building:\n\t%s\n' % '\n\t'.join(conf.env.DROBILLAD_BUILD))

    not_building = []
    for i in projects:
        if i not in conf.env.DROBILLAD_BUILD:
            not_building += [i]

    if not_building != []:
        Logs.warn('Not building:\n\t%s\n' % '\n\t'.join(not_building))

def source_tree_env():
    "Set up the environment to run things from the source tree."
    env          = os.environ
    library_path = []
    if 'LD_LIBRARY_PATH' in env:
        library_path = env.LD_LIBRARY_PATH.split(os.pathsep)
    for i in 'serd sord sratom lilv suil raul ganv'.split():
        library_path += [ os.path.join(os.getcwd(), 'build', i) ]

    ingen_module_path = []
    for i in 'client server shared gui serialisation'.split():
        path = os.path.join(os.getcwd(), 'build', 'ingen', 'src', i)
        library_path      += [ path ]
        ingen_module_path += [ path ]

    env.LD_LIBRARY_PATH   = os.pathsep.join(library_path)
    env.INGEN_MODULE_PATH = os.pathsep.join(ingen_module_path)
    env.LV2_PATH = os.pathsep.join(['~/.lv2', os.path.join(os.getcwd(), 'lv2')])
    return env

def run(ctx):
    if not Options.options.cmd:
        Logs.error("missing --cmd option for run command")
        return

    cmd = Options.options.cmd
    Logs.pprint('GREEN', 'Running %s' % cmd)

    subprocess.call(cmd, shell=True, env=source_tree_env())

def build(bld):
    autowaf.set_recursive()
    for i in bld.env.DROBILLAD_BUILD:
        bld.recurse(i)

def test(ctx):
    autowaf.set_recursive();
    os.environ = source_tree_env()
    for i in ['raul', 'serd', 'sord', 'sratom', 'lilv']:
        ctx.recurse(i)
