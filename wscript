#!/usr/bin/env python
# wscript for Mechanic
# Mariusz Slonina, 2010
# encoding: utf-8

import sys
import subprocess
import os
import Utils
import Build
import Options
import Environment
import Scripting

import string

Scripting.g_gz = 'gz'

VERSION='0.12.0-UNSTABLE-2-3'
APPNAME='mechanic'
URL='http://mechanics.astri.umk.pl/projects/mechanic'
BUGS='mariusz.slonina@gmail.com'
AUTHOR='Mariusz Slonina, NCU'

srcdir = '.'
blddir = 'build'
tooldir = srcdir + '/waf-tools/'

# Sources
core = ['src', 'src/core', 'src/modes', 'src/modules']
fortran = ['src/fortran']

all_modules = ['src/modules/hello',
               'src/modules/echo',
               'src/modules/mandelbrot',
               ]
all_fortran_modules = ['src/modules/modules_fortran/map',
                       'src/modules/modules_fortran/fhello'
                       ]

all_engines = ['src/engines/odex',
               'src/engines/taylor',
               'src/engines/gpu']

all_libs = ['src/libs/orbit']

# Standard headers
stdheads = ['stdio.h', 'locale.h', 'memory.h', 'stdlib.h', 'stdint.h', 'inttypes.h', 
           'strings.h', 'string.h', 'sys/stat.h', 'sys/types.h', 'unistd.h',
					 'mpi.h']

# Standard functions
stdfunc = [['printf', 'stdio.h', 1],
					 ['memmove', 'memory.h', 1],
           ['strcspn', 'string.h', 1],
           ['strspn', 'string.h', 1],
           ['strstr', 'string.h', 1],
           ['strtol', 'stdlib.h', 1],
           ['strtok', 'string.h', 1],
					 ]

# Standard types
stdtypes = [['ssize_t', 'sys/types.h', 1],
            ['int', 'sys/types.h', 1],
            ['char', 'sys/types.h', 1],
            ['double', 'sys/types.h', 1]
            ]


#
# [Helper] Check for standard headers/functions
#
def _check_std_headers(conf, heads, funcs, types):
  for i in heads:
    conf.check_cc(header_name=i, mandatory=True)

  for i in funcs:
    conf.check_cc(function_name=i[0], header_name=i[1], mandatory=i[2])
  
  for i in types:
    conf.check_cc(type_name=i[0], header_name=i[1], mandatory=i[2])

#
# [Helper] Check for library
#
def _check_lib(conf, l):
  var = l[0].upper()
  try:
    conf.check_cc(lib=l[0], uselib_store=var, mandatory=l[5],
      msg="Checking for "+l[0]+" library")
  except:
    Utils.pprint('RED', l[0] + " library was not found on your system")

  try:
    conf.check_cc(header_name=l[1], uselib=var, mandatory=l[5])
  except:
    Utils.pprint('RED', l[1] + " header was not found on your system :(")
 
  if not len(l[3]) == 0:
    try:
      conf.check_cc(fragment=l[3], 
                    execute=True, 
                    uselib=var + ' ' + l[4], 
                    define_ret=True, 
                    mandatory=l[5], 
                    msg="Checking if " + l[0] + " is usable"
                    )
    except:
      Utils.pprint('RED', l[0] + " is not usable :(")

#
# [Helper] Process optional modules/engines/libs
#
def _process_optionals(option, opt, value, parser):
  vals = value.split(',')
  if vals == ['']:
    vals = []
  if getattr(parser.values, option.dest):
    vals += getattr(parser.values, option.dest)
  setattr(parser.values, option.dest, vals)

#
# [Helper] Check if given optional module/engine/lib exists
#
def _check_optional(options, type):

  global all_modules
  global all_fortran_modules
  global all_engines
  global all_libs

  all = []
  notfound = []
  to_build = []
  op = ''
  subdir = ''

  if type == 'modules':
    all = all_modules

  if type == 'fortran_modules':
    type = 'modules'
    subdir = 'modules_fortran/'
    all = all_fortran_modules
  
  if type == 'engines':
    all = all_engines
  
  if type == 'libs':
    all = all_libs

  pref = 'src/' + type + '/' + subdir

  for op in options:
    op = pref + op
    try:
      all.index(op)
      to_build.append(op)
    except:
      notfound.append(os.path.basename(op))

  if notfound:
    Utils.pprint('RED', type[:-1].upper() + " not found: " + ", ".join(notfound))
    raise sys.exit(1)

  return to_build

#
# [Helper] Configure optional modules/engines/libs
#
def _configure_optionals(conf, type):

  var = []
  
  if type == 'modules':
    if Options.options.with_modules != None:
      var = Options.options.with_modules
  
  if type == 'fortran_modules':
    if Options.options.with_fortran_modules != None:
      var = Options.options.with_fortran_modules

  if type == 'engines':
    if Options.options.with_engines != None:
      var = Options.options.with_engines
    
  if type == 'libs':
    if Options.options.with_libs != None:
      var = Options.options.with_libs
   
  to_build = _check_optional(var, type)

  return to_build

#
# [Helper] List modules/engines/libs when waf --help
#
def _list_options(opts):
  list = []
  for a in opts:
    list.append(os.path.basename(a))

  return list

#
# [Helper] Open test code file and return its contents
#
def _test_code(code):

  buffer = ''
  f = open(code, 'ro');
  
  for i in f:
    buffer = buffer + i

  f.close

  return buffer

def _join_list(list):
  
  aa = []
  for i in list:
    aa.append(os.path.basename(i))

  buffer = ', '.join(aa)
  return buffer

def _join_or_say_none(list):
  buffer = _join_list(list)
  if not buffer:
    buffer = 'None'
  return buffer

def _say_yes_or_no(value):
  if value == 1:
    return 'Yes'
  else:
    return 'No'
    

def _configure_summary(conf):

  print
  Utils.pprint('YELLOW', APPNAME.upper() + ' ' + VERSION.upper() + ' setup:')
  Utils.pprint('YELLOW', '  Installation path: ' + conf.env['PREFIX'])
  
  Utils.pprint('YELLOW',
  '  Additional modules: ' + _join_or_say_none(conf.env['MECHANIC_BUILD_MODULES']))
  Utils.pprint('YELLOW',
  '  Additional engines: ' + _join_or_say_none(conf.env['MECHANIC_BUILD_ENGINES']))
  Utils.pprint('YELLOW',
  '  Additional libs: ' + _join_or_say_none(conf.env['MECHANIC_BUILD_LIBS']))
  Utils.pprint('YELLOW',
  '  F2003 bindings: ' + _say_yes_or_no(conf.env['MPIF']))
  Utils.pprint('YELLOW',
  '  F2003 modules: ' + _join_or_say_none(conf.env['MECHANIC_BUILD_F2003_MODULES']))
  print

#
# SET OPTIONS
#
def set_options(opt):

  global all_modules, all_libs, all_engines
  list = []

  opt.tool_options('compiler_mpicc', tooldir=tooldir)
  opt.tool_options('compiler_mpif90', tooldir=tooldir)
  
  opt.add_option('--with-fortran', action = 'store_true', default = False,
                  help = 'Build Fortran2003 bindings', dest = 'with_f2003')

  opt.add_option('--with-doc', action = 'store_true', default = False,
                  help = 'Build documentation', dest = 'with_doc')

  list = _list_options(all_libs)
  opt.add_option('--with-libs', action = 'callback', callback = _process_optionals,
                  type = 'string', default = None,
                  help = 'Build libs, options: ' + ', '.join(list), dest = 'with_libs')

  list = _list_options(all_engines)
  opt.add_option('--with-engines', action = 'callback', callback = _process_optionals,
                  type = 'string', default = None,
                  help = 'Build engines, options: ' + ', '.join(list), dest = 'with_engines')

  list = _list_options(all_modules)
  opt.add_option('--with-modules', action = 'callback', callback = _process_optionals,
                  type = 'string', default = None,
                  help = 'Build modules, options: ' + ', '.join(list), dest = 'with_modules')

  list = _list_options(all_fortran_modules)
  opt.add_option('--with-fortran-modules', action = 'callback', callback = _process_optionals,
                  type = 'string', default = None,
                  help = 'Build F2003 modules, options: ' + ', '.join(list),
                  dest = 'with_fortran_modules')
#
# CONFIGURE
#
def configure(conf):

  global stdheads
  global stdfunc
  global stdtypes
  global hdf5_test_code
  global core
  global fortran
  
  conf.env['TOOLDIR'] = srcdir + '/waf-tools/'
  conf.env['MECHANIC_CORE'] = []
  conf.env['MECHANIC_BUILD_FORTRAN'] = []
  conf.env['MECHANIC_BUILD_MODULES'] = []
  conf.env['MECHANIC_BUILD_F2003_MODULES'] = []
  conf.env['MECHANIC_BUILD_ENGINES'] = []
  conf.env['MECHANIC_BUILD_LIBS'] = []
  conf.env['MECHANIC_BUILD_DOC'] = []

  test_code_libm = _test_code('waf-tests/test-m.c')
  test_code_popt = _test_code('waf-tests/test-popt.c')
  test_code_dl = _test_code('waf-tests/test-dl.c')
  test_code_hdf = _test_code('waf-tests/test-hdf.c')
  test_code_lrc = _test_code('waf-tests/test-lrc.c')

  if Options.options.with_f2003:
    conf.env.MPIF = 1

  if Options.options.with_fortran_modules and not Options.options.with_f2003:
    Utils.pprint('YELLOW','Fortran modules used, adding F2003 bindings support')
    conf.env.MPIF = 1

  # Standard checks
  conf.check_tool('compiler_mpicc', tooldir=tooldir)
  
  if conf.env.MPIF:
    conf.check_tool('compiler_mpif90', tooldir=tooldir)

  _check_std_headers(conf, stdheads, stdfunc, stdtypes)
  _check_lib(conf, ['m', 'math.h', 'pow', test_code_libm, '', 1])
  _check_lib(conf, ['hdf5', 'hdf5.h', 'H5Dopen2', test_code_hdf, '', 1])
  _check_lib(conf, ['readconfig', 'libreadconfig_hdf5.h', 'LRC_HDF5Writer',
                    test_code_lrc, 'HDF5', 1])
  _check_lib(conf, ['dl', 'dlfcn.h', 'dlopen', test_code_dl, '', 1])
  _check_lib(conf, ['popt', 'popt.h', 'poptGetContext', test_code_popt, '', 1])
	
	# Check for doxygen
  if Options.options.with_doc:
    try:
      conf.find_program('doxygen', var='DOXYGEN', mandatory=True)
      conf.find_program('pdflatex', var='PDFLATEX', mandatory=True)
      conf.env.BUILDDOC = 1
      conf.env['MECHANIC_BUILD_DOC'] = ['doc']
    except:
      print "You need Doxygen and PdfLatex installed to build documentation"

  # Subdirs
  conf.env['MECHANIC_CORE'] = core
  conf.env['MECHANIC_BUILD_MODULES'] = _configure_optionals(conf, 'modules')
  conf.env['MECHANIC_BUILD_ENGINES'] = _configure_optionals(conf, 'engines')
  conf.env['MECHANIC_BUILD_LIBS'] = _configure_optionals(conf, 'libs')
  
  if conf.env.MPIF:
    conf.env['MECHANIC_BUILD_FORTRAN'] = fortran
    conf.env['MECHANIC_BUILD_F2003_MODULES'] = _configure_optionals(conf, 'fortran_modules')
	
  # Define standard declarations
  conf.define('PACKAGE_NAME', APPNAME)
  conf.define('PACKAGE_VERSION', VERSION)
  conf.define('PACKAGE_AUTHOR', AUTHOR)
  conf.define('PACKAGE_BUGREPORT', BUGS)
  conf.define('PACKAGE_URL', URL)

  # Compiler flags
  conf.env['CCFLAGS'] += ['-Wall', '-g', '-ggdb']
  conf.env['CCFLAGS'] += ['-std=c99']
  conf.env['CCFLAGS'] += ['-fpic']
  conf.env['CCFLAGS'] += ['-Dpic']
  conf.env['CPPFLAGS'] += ['-DHAVE_CONFIG_H']
  conf.env['CPPFLAGS'] += ['-I../build/default']

  if conf.env.MPIF:
    conf.env['FCFLAGS'] += ['-g', '-ggdb']
    conf.env['FCFLAGS'] += ['-std=f2003']
    conf.env['FCFLAGS'] += ['-fpic']
    conf.env['FCFLAGS'] += ['-Dpic']
    # necessary to put the module in right place after compilation
    conf.env['FCFLAGS'] += ['-Jdefault/src/fortran'] 


  # Write config.h
  conf.write_config_header('config.h')

  # Print summary
  _configure_summary(conf)

#
# BUILD
#
def build(bld):
  bld.use_the_magic() # <- use this so that we have proper chain in F2003
  bld.add_subdirs(bld.env['MECHANIC_CORE'])
  bld.add_subdirs(bld.env['MECHANIC_BUILD_FORTRAN'])
  bld.add_subdirs(bld.env['MECHANIC_BUILD_MODULES'])
  bld.add_subdirs(bld.env['MECHANIC_BUILD_F2003_MODULES'])
  bld.add_subdirs(bld.env['MECHANIC_BUILD_ENGINES'])
  bld.add_subdirs(bld.env['MECHANIC_BUILD_LIBS'])
  bld.add_subdirs(bld.env['MECHANIC_BUILD_DOC'])
  
  
