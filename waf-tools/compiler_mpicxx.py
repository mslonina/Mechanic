#! /usr/bin/env python
# encoding: utf-8

import os,sys,imp,types,ccroot
import optparse
import Utils,Configure,Options
from Logs import debug

tooldir = './waf-tools'
mpicxx_list = ['mpic++', 'mpicxx', 'mpiCC']
mpicxx_compiler={'win32':mpicxx_list,'cygwin':mpicxx_list,'darwin':mpicxx_list,'aix':mpicxx_list,'linux':mpicxx_list,'sunos':mpicxx_list,'irix':mpicxx_list,'hpux':mpicxx_list,'default':['mpic++']}
def __list_possible_compiler(platform):
	try:
		return mpicxx_compiler[platform]
	except KeyError:
		return mpicxx_compiler["default"]
def detect(conf):
	try:test_for_compiler=Options.options.check_mpicxx_compiler
	except AttributeError:raise Configure.ConfigurationError("Add set_options(opt): opt.tool_options('compiler_mpicxx')")
	orig=conf.env
	for compiler in test_for_compiler.split():
		try:
			conf.env=orig.copy()
			conf.check_tool(compiler, tooldir=tooldir)
		except Configure.ConfigurationError,e:
			debug('compiler_mpicxx: %r'%e)
		else:
			if conf.env['CXX']:
				orig.table=conf.env.get_merged_dict()
				conf.env=orig
				conf.check_message(compiler,'',True)
				conf.env['COMPILER_CXX']=compiler
				break
			conf.check_message(compiler,'',False)
			break
	else:
		conf.fatal('could not configure a mpicxx compiler!')
def set_options(opt):
	build_platform=Utils.unversioned_sys_platform()
	possible_compiler_list=__list_possible_compiler(build_platform)
	test_for_compiler=' '.join(possible_compiler_list)
	mpicxx_compiler_opts=opt.add_option_group('MPIC++ Compiler Options')
	mpicxx_compiler_opts.add_option('--check-mpicxx-compiler',default="%s"%test_for_compiler,help='On this platform (%s) the following C++ Compiler will be checked by default: "%s"'%(build_platform,test_for_compiler),dest="check_mpicxx_compiler")
	for mpicxx_compiler in test_for_compiler.split():
		opt.tool_options('%s'%mpicxx_compiler,option_group=mpicxx_compiler_opts,tooldir=tooldir)

