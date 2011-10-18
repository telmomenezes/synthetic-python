#!/usr/bin/env python
# encoding: utf-8

"""
setup.py

Created by Telmo Menezes on 2011-04-15.
Copyright (c) 2011 Telmo Menezes. All rights reserved.
"""

from distutils.core import setup, Extension

core_module = Extension('core',
                    define_macros = [('MAJOR_VERSION', '1'),
                                     ('MINOR_VERSION', '0')],
                    libraries = ['m'],
                    include_dirs = ['src', 'src/gp'],
                    sources = ['src/synpython.c', 'src/edge.c', 'src/node.c', 'src/emd.c', 'src/drmap.c', 'src/network.c', 'src/mgenerator.c', 'src/gp/gpnode.c', 'src/gp/gptree.c'])

setup (name = 'synthetic',
       version = '1.0',
       description = 'TBD',
       author = 'Telmo Menezes',
       author_email = 'telmo@telmomenezes.com',
       url = 'http://telmomenezes.com',
       packages = ['syn'],
       package_dir = {'syn': 'syn'},
       long_description = '''
TBD.
''',
       ext_package = 'syn',
       ext_modules = [core_module])
