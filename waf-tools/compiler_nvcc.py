#! /usr/bin/env python
# encoding: utf-8

import os,sys,imp,types,ccroot
import optparse
import Utils,Configure,Options
from Logs import debug

tooldir = './waf-tools'
nvcc_compiler={'win32':['nvcc'],'cygwin':['nvcc'],'darwin':['nvcc'],'aix':['nvcc'],'linux':['nvcc'],'sunos':['nvcc'],'irix':['nvcc'],'hpux':['nvcc'],'default':['nvcc']}
def __list_possible_compiler(platform):
	try:
		return nvcc_compiler[platform]
	except KeyError:
		return nvcc_compiler["default"]
def detect(conf):
	try:test_for_compiler=Options.options.check_nvcc_compiler
	except AttributeError:conf.fatal("Add set_options(opt): opt.tool_options('compiler_nvcc')")
	orig=conf.env
	for compiler in test_for_compiler.split():
		conf.env=orig.copy()
		try:
			conf.check_tool(compiler, tooldir=tooldir)
		except Configure.ConfigurationError,e:
			debug('compiler_nvcc: %r'%e)
		else:
			if conf.env['CC']:
				orig.table=conf.env.get_merged_dict()
				conf.env=orig
				conf.check_message(compiler,'',True)
				conf.env['COMPILER_CC']=compiler
				break
			conf.check_message(compiler,'',False)
			break
	else:
		conf.fatal('could not configure a mpi compiler!')
def set_options(opt):
  build_platform=Utils.unversioned_sys_platform()
  possible_compiler_list=__list_possible_compiler(build_platform)
  test_for_compiler=' '.join(possible_compiler_list)
  nvcc_compiler_opts=opt.add_option_group("NVCC Compiler Options")
  nvcc_compiler_opts.add_option('--check-nvcc-compiler',default="%s"%test_for_compiler,help='On this platform (%s) the following NVCC-Compiler will be checked by default: "%s"'%(build_platform,test_for_compiler),dest="check_nvcc_compiler")
  for nvcc_compiler in test_for_compiler.split():
    opt.tool_options('%s'%nvcc_compiler,option_group=nvcc_compiler_opts, tooldir=tooldir)

