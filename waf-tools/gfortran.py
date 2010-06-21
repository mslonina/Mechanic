#! /usr/bin/env python
# encoding: utf-8

import ccroot # <- leave this
import fortran
from Configure import conftest

@conftest
def find_gfortran(conf):
	v = conf.env
	fc = conf.find_program('gfortran', var='FC')
	if not fc: 
		conf.fatal('gfortran not found')
	v['FC_NAME'] = 'GFORTRAN'
	v['FC'] = fc

@conftest
def gfortran_flags(conf):
	v = conf.env

	v['FC_SRC_F']    = ''
	v['FC_TGT_F']    = ['-c', '-o', ''] # shell hack for -MD
	v['FCPATH_ST']  = '-I%s' # template for adding include paths
	v['FORTRANMODFLAG']  = ['-M', ''] # template for module path

	# linker
	if not v['LINK_FC']: v['LINK_FC'] = v['FC']
	v['FCLNK_SRC_F'] = ''
	v['FCLNK_TGT_F'] = ['-o', ''] # shell hack for -MD

	v['FCFLAGS_DEBUG'] = ['-Werror']

	# shared library: XXX this is platform dependent, actually (no -fPIC on
	# windows, etc...)
	v['shlib_FCFLAGS'] = ['-fPIC']
	v['shlib_LINKFLAGS'] = ['-shared']
	#v['shlib_PATTERN']       = 'lib%s.so'

@conftest
def detect(conf):
	find_gfortran(conf)
	gfortran_flags(conf)
