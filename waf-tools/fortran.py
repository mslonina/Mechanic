#! /usr/bin/env python
# encoding: utf-8
# WAF playground, adjusted for Mechanic
import re

import Utils, Task, TaskGen, Logs
import ccroot # <- leave this
import config_fortran # <- leave this
from TaskGen import feature, before, after, extension
from Configure import conftest, conf
import Build
import Options

    
INCLUDE_REGEX = """(?:^|['">]\s*;)\s*INCLUDE\s+(?:\w+_)?[<"'](.+?)(?=["'>])"""
USE_REGEX = """(?:^|;)\s*USE(?:\s+|(?:(?:\s*,\s*(?:NON_)?INTRINSIC)?\s*::))\s*(\w+)"""

EXT_MOD = ".mod"
EXT_FC = ".F03"
EXT_FCPP = ".F"
EXT_OBJ = ".o"

# TODO:
#   - handle pre-processed files (FORTRANPPCOM in scons)
#   - handle modules
#   - handle multiple dialects
#   - windows...

class fortran_parser(object):
  def __init__(self, env, incpaths, modsearchpath):
    self.allnames = []

    self.re_inc = re.compile(INCLUDE_REGEX, re.IGNORECASE)
    self.re_use = re.compile(USE_REGEX, re.IGNORECASE)

    self.env = env

    self.nodes = []
    self.names = []
    self.modules = []

    self.incpaths = incpaths
    # XXX: 
    self.modsearchpath = modsearchpath

  def tryfind_header(self, filename):
    found = 0
    for n in self.incpaths:
      found = n.find_resource(filename)
      if found:
        self.nodes.append(found)
        self.waiting.append(found)
        break
    if not found:
      if not filename in self.names:
        self.names.append(filename)

  def tryfind_module(self, filename):
    found = 0
    for n in self.modsearchpath:
      found = n.find_resource(filename + EXT_MOD)
      if found:
        self.nodes.append(found)
        self.waiting.append(found)
        break
    if not found:
      if not filename in self.names:
        self.names.append(filename)

  def find_deps(self, code):
    headers = []
    modules = []
    for line in code.readlines():
      m = self.re_inc.search(line)
      if m:
        headers.append(m.group(1))
      m = self.re_use.search(line)
      if m:
        modules.append(m.group(1))
    return headers, modules

  def start(self, node):
    self.waiting = [node]
    # while the stack is not empty, add the dependencies
    while self.waiting:
      nd = self.waiting.pop(0)
      self.iter(nd)

  def iter(self, node):
    path = node.abspath(self.env) # obtain the absolute path
    code = open(path, 'r')
    hnames, mnames = self.find_deps(code)
    for x in hnames:
      # optimization
      if x in self.allnames: 
        continue
      self.allnames.append(x)

      # for each name, see if it is like a node or not
      self.tryfind_header(x)

    for x in mnames:
      # optimization
      if x in self.allnames: 
        continue
      self.allnames.append(x)

      # for each name, see if it is like a node or not
      self.tryfind_module(x)

def scan(self):
  env = self.env
  gruik = fortran_parser(env, env['INC_PATHS'], env["MODULE_SEARCH_PATH"])
  gruik.start(self.inputs[0])

  #print self.inputs, gruik.nodes, gruik.names
  if Logs.verbose:
    Logs.debug('deps: nodes found for %s: %s %s' % (str(self.inputs[0]), str(gruik.nodes), str(gruik.names)))
    #debug("deps found for %s: %s" % (str(node), str(gruik.deps)), 'deps')
  return (gruik.nodes, gruik.names)

#################################################### Task definitions

def fortran_compile(task):
  env = task.env
  def tolist(xx):
    if isinstance(xx, str):
      return [xx]
    return xx
  cmd = []
  cmd.extend(tolist(env["FC"]))
  cmd.extend(tolist(env["FCFLAGS"]))
  cmd.extend(tolist(env["_FCINCFLAGS"]))
  cmd.extend(tolist(env["_FCMODOUTFLAGS"]))
  for a in task.outputs:
    cmd.extend(tolist(env["FC_TGT_F"] + tolist(a.bldpath(env))))
  for a in task.inputs:
    cmd.extend(tolist(env["FC_SRC_F"]) + tolist(a.srcpath(env)))
  cmd = [x for x in cmd if x]
  cmd = [cmd]
  
  ret = task.exec_command(*cmd)
  return ret

fcompiler = Task.task_type_from_func('fortran',
  vars=["FC", "FCFLAGS", "_FCINCFLAGS", "FC_TGT_F", "FC_SRC_F",
    "FORTRANMODPATHFLAG"],
  func=fortran_compile,
  color='GREEN',
  ext_out=EXT_OBJ,
  ext_in=EXT_FC)
fcompiler.scan = scan

# Module compiler
def fortran_module_compile(task):
  env = task.env
  def tolist(xx):
    if isinstance(xx, str):
      return [xx]
    return xx
  cmd = []
  cmd.extend(tolist(env["FC"]))
  cmd.extend(tolist(env["FCFLAGS"]))
  cmd.extend(tolist(env["_FCINCFLAGS"]))
  cmd.extend(tolist(env["_FCMODOUTFLAGS"]))
  for a in task.outputs:
    cmd.extend(tolist('-c')) # F modules cannot be build with -o
  for a in task.inputs:
    cmd.extend(tolist(env["FC_SRC_F"]) + tolist(a.srcpath(env)))
  cmd = [x for x in cmd if x]
  cmd = [cmd]

  ret = task.exec_command(*cmd)
  return ret

fcompiler_module = Task.task_type_from_func('fmodule',
  vars=["FC", "FCFLAGS", "_FCINCFLAGS", "FC_SRC_F", "FC_TGT_F"
    "FORTRANMODPATHFLAG"],
  func=fortran_module_compile,
  color='GREEN',
  ext_in=EXT_FC)
fcompiler_module.scan = scan

# Task to compile fortran source which needs to be preprocessed by cpp first
Task.simple_task_type('fortranpp',
  '${FC} ${FCFLAGS} ${CPPFLAGS} ${_CCINCFLAGS} ${_CCDEFFLAGS} ${FC_TGT_F}${TGT} ${FC_SRC_F}${SRC} ',
  'GREEN',
  ext_out=EXT_OBJ,
  ext_in=EXT_FCPP)

Task.simple_task_type('fortran_link',
  '${FC} ${FCLNK_SRC_F}${SRC} ${FCLNK_TGT_F}${TGT} ${LINKFLAGS}',
  color='YELLOW', ext_in=EXT_OBJ)

@extension(EXT_FC)
def fortran_hook(self, node):
  obj_ext = '_%d.o' % self.idx
  
  for x in self.features:
    if x in ['fmodule']:
      obj_ext = EXT_MOD
      break

  if obj_ext == EXT_MOD:
    task = self.create_task('fmodule')
  else:
    task = self.create_task('fortran')
    
  task.inputs = [node]
  task.outputs = [node.change_ext(obj_ext)]
  self.compiled_tasks.append(task)
  return task

@extension(EXT_FCPP)
def fortranpp_hook(self, node):
  obj_ext = '_%d.o' % self.idx

  task = self.create_task('fortranpp')
  task.inputs = [node]
  task.outputs = [node.change_ext(obj_ext)]
  self.compiled_tasks.append(task)
  return task

#################################################### Task generators

# we reuse a lot of code from ccroot.py

FORTRAN = 'init_f default_cc apply_incpaths apply_defines_cc apply_type_vars apply_lib_vars add_extra_flags apply_obj_vars_cc'.split()
FPROGRAM = 'apply_verif vars_target_cprogram apply_objdeps apply_obj_vars '.split()
FSHLIB = 'apply_verif apply_objdeps apply_obj_vars apply_vnum'.split()
FMODULE = 'apply_verif apply_objdeps apply_obj_vars apply_vnum'.split()
FSTATICLIB = 'apply_verif apply_objdeps apply_obj_vars '.split()

TaskGen.bind_feature('fortran', FORTRAN)
TaskGen.bind_feature('fprogram', FPROGRAM)
TaskGen.bind_feature('fshlib', FSHLIB)
TaskGen.bind_feature('fmodule', FMODULE)
TaskGen.bind_feature('fstaticlib', FSTATICLIB)

TaskGen.declare_order('init_f', 'apply_lib_vars')
TaskGen.declare_order('default_cc', 'apply_core')

@feature('fortran')
@before('apply_type_vars')
@after('default_cc', 'cprogram')
def init_f(self):
  # PATH flags:
  # - CPPPATH: same as C, for pre-processed fortran files
  # - FORTRANMODPATH: where to look for modules (.mod)
  # - FORTRANMODOUTPATH: where to *put* modules (.mod)
  self.p_flag_vars = ['FC', 'FCFLAGS', 'RPATH', 'LINKFLAGS',
      'FORTRANMODPATH', 'CPPPATH', 'FORTRANMODOUTPATH', '_CCINCFLAGS']
  self.p_type_vars = ['FCFLAGS', 'LINKFLAGS']

@feature('fortran')
@after('apply_incpaths', 'apply_obj_vars_cc')
def apply_fortran_type_vars(self):
  for x in self.features:
    if not x in ['fprogram', 'fstaticlib', 'fshlib', 'fmodule']:
      continue
    x = x.lstrip('f')

    # if the type defines uselib to add, add them
    st = self.env[x + '_USELIB']
    if st: self.uselib = self.uselib + ' ' + st

    # each compiler defines variables like 'shlib_FCFLAGS', 'shlib_LINKFLAGS', etc
    # so when we make a task generator of the type shlib, FCFLAGS are modified accordingly
    for var in self.p_type_vars:
      compvar = '%s_%s' % (x, var)
      value = self.env[compvar]
      if value: self.env.append_value(var, value)

  # Put module and header search paths into _FCINCFLAGS
  app = self.env.append_unique
  for i in self.env["FORTRANMODPATH"]:
    app('_FCINCFLAGS', self.env['FCPATH_ST'] % i)

  for i in self.env["_CCINCFLAGS"]:
    app('_FCINCFLAGS', i)

  #opath = self.env["FORTRANMODOUTPATH"]
  #if not opath:
  # self.env["_FCMODOUTFLAGS"] = self.env["FORTRANMODFLAG"] + opath
  # app('_FCINCFLAGS', self.env['FCPATH_ST'] % opath)
  #else:
  # # XXX: assume that compiler put .mod in cwd by default
  # app('_FCINCFLAGS', self.env['FCPATH_ST'] % self.bld.bdir)

@feature('fprogram', 'fshlib', 'fstaticlib')
@after('apply_core')
@before('apply_link', 'apply_lib_vars')
def apply_fortran_link(self):
  # override the normal apply_link with c or c++ - just in case cprogram is given too
  try: self.meths.remove('apply_link')
  except ValueError: pass

  link = 'fortran_link'
  if 'fstaticlib' in self.features:
    link = 'ar_link_static'

  def get_name():
    if 'fprogram' in self.features:
      return '%s'
    elif 'fshlib' in self.features:
      if cmp(Options.platform, 'darwin') == 0:
        return 'lib%s.dylib'
      if cmp(Options.platform, 'linux') == 0:
        return 'lib%s.so'
    else:
      return 'lib%s.a'

  linktask = self.create_task(link)
  outputs = [t.outputs[0] for t in self.compiled_tasks]
  linktask.set_inputs(outputs)
  linktask.set_outputs(self.path.find_or_declare(get_name() % self.target))
  linktask.chmod = self.chmod

  self.link_task = linktask

#################################################### Configuration

@conf
def check_fortran(self, *k, **kw):
  if not 'compile_filename' in kw:
    kw['compile_filename'] = 'test.f'
  if 'fragment' in kw:
    kw['code'] = kw['fragment']
  if not 'code' in kw:
    kw['code'] = '''        program main
        end     program main
'''

  if not 'compile_mode' in kw:
    kw['compile_mode'] = 'fortran'
  if not 'type' in kw:
    kw['type'] = 'fprogram'
  if not 'env' in kw:
    kw['env'] = self.env.copy()
  kw['execute'] = kw.get('execute', None)

  kw['msg'] = kw.get('msg', 'Compiling a simple fortran app')
  kw['okmsg'] = kw.get('okmsg', 'ok')
  kw['errmsg'] = kw.get('errmsg', 'bad luck')

  self.check_message_1(kw['msg'])
  ret = self.run_c_code(*k, **kw) == 0
  if not ret:
    self.check_message_2(kw['errmsg'], 'YELLOW')
  else:
    self.check_message_2(kw['okmsg'], 'GREEN')

  return ret

#################################################### Add some flags on some feature

@feature('flink_with_c++')
@after('apply_core')
@before('apply_link', 'apply_lib_vars', 'apply_fortran_link')
def apply_special_link(self):
  linktask = self.create_task('fortran_link')
  outputs = [t.outputs[0] for t in self.compiled_tasks]
  linktask.set_inputs(outputs)
  linktask.set_outputs(self.path.find_or_declare("and_without_target"))
  linktask.chmod = self.chmod
  self.link_task = linktask

@feature('flink_with_c++')
@before('apply_lib_vars')
@after('default_cc')
def add_some_uselib_vars(self):
  #if sys.platform == ...
  self.uselib += ' DEBUG'
