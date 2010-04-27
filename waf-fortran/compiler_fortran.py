#!/usr/bin/env python
# encoding: utf-8
#
# WAF PLAYGROUND

import os, sys, imp, types, ccroot
import optparse
import Utils, Configure, Options

fortran_compiler = {
  'darwin': ['gfortran', 'ifort'],
  'linux': ['gfortran', 'ifort'],
  'default': ['gfortran']
}

def __list_possible_compiler(platform):
  try:
    return(fortran_compiler[platform])
  except KeyError:
    return(fortran_compiler["default"])

def detect(conf):
  try: 
    test_for_compiler = Options.options.check_fortran_compiler
  except AttributeError: 
    raise Configure.ConfigurationError(
        "Add set_options(opt): opt.tool_options('compiler_fortran')")
  for compiler in test_for_compiler.split():
    try:
      conf.check_tool(compiler, tooldir='.')
    except Configure.ConfigurationError:
      pass
    else:
      if conf.env['FC']:
        conf.check_message("%s" % compiler, '', True)
        conf.env["COMPILER_FC"] = "%s" % compiler
        return
      conf.check_message("%s" %fortran_compiler, '', False)
      break
  conf.env["COMPILER_FC"] = None

def set_options(opt):
  detected_platform = Options.platform
  possible_compiler_list = __list_possible_compiler(detected_platform)
  test_for_compiler = str(" ").join(possible_compiler_list)
  fortran_compiler_opts = opt.add_option_group("Fortran Compiler Options")
  fortran_compiler_opts.add_option('--check-fortran-compiler', 
      default="%s" % test_for_compiler,
      help='On this platform (%s) the following Fortran Compiler will be checked by default: "%s"' % (detected_platform, test_for_compiler),
    dest="check_fortran_compiler")

  for compiler in test_for_compiler.split():
    opt.tool_options('%s' % compiler, option_group=fortran_compiler_opts)
