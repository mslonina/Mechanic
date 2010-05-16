#! /usr/bin/env python
# encoding: utf-8

import os,sys,imp,types,ccroot
import optparse
import Utils,Configure,Options
from Logs import debug
mpi_c_compiler={'win32':['mpicc'],'cygwin':['mpicc'],'darwin':['mpicc'],'aix':['mpicc'],'linux':['mpicc'],'sunos':['mpicc'],'irix':['mpicc'],'hpux':['mpicc'],'default':['mpicc']}
def __list_possible_compiler(platform):
	try:
		return mpi_c_compiler[platform]
	except KeyError:
		return mpi_c_compiler["default"]
def detect(conf):
	try:test_for_compiler=Options.options.check_mpi_c_compiler
	except AttributeError:conf.fatal("Add set_options(opt): opt.tool_options('compiler_mpicc')")
	orig=conf.env
	for compiler in test_for_compiler.split():
		conf.env=orig.copy()
		try:
			conf.check_tool(compiler, tooldir='./waf-tools/mpi')
		except Configure.ConfigurationError,e:
			debug('compiler_mpicc: %r'%e)
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
  mpi_cc_compiler_opts=opt.add_option_group("MPICC Compiler Options")
  mpi_cc_compiler_opts.add_option('--check-mpicc-compiler',default="%s"%test_for_compiler,help='On this platform (%s) the following MPI C-Compiler will be checked by default: "%s"'%(build_platform,test_for_compiler),dest="check_mpi_c_compiler")
  for mpi_c_compiler in test_for_compiler.split():
    opt.tool_options('%s'%mpi_c_compiler,option_group=mpi_cc_compiler_opts,
    tooldir='./waf-tools/mpi')

