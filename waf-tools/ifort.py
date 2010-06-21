#! /usr/bin/env python
# encoding: utf-8

import ccroot # <- leave this
import fortran
import ar
from Configure import conftest

@conftest
def find_ifort(conf):
	v = conf.env
	fc = conf.find_program('ifort', var='FC')
	if not fc: 
		conf.fatal('ifort not found')
	v['FC_NAME'] = 'IFORT'
	v['FC'] = fc

@conftest
def ifort_flags(conf):
	v = conf.env

	v['FC_SRC_F']    = ''
	v['FC_TGT_F']    = ['-c', '-o', ''] # shell hack for -MD
	v['FCPATH_ST']  = '-I%s' # template for adding include paths

	# linker
	if not v['LINK_FC']: v['LINK_FC'] = v['FC']
	v['FCLNK_SRC_F'] = ''
	v['FCLNK_TGT_F'] = ['-o', ''] # shell hack for -MD

	#v['FCFLAGS_DEBUG'] = []

	# shared library: XXX this is platform dependent, actually (no -fPIC on
	# windows, etc...)
	v['shlib_FCFLAGS'] = ['-fpic']
	v['shlib_LINKFLAGS'] = ['-shared']
	#v['shlib_PATTERN']       = 'lib%s.so'

def detect(conf):
	v = conf.env
	find_ifort(conf)
	ar.find_ar(conf)
	ifort_flags(conf)
