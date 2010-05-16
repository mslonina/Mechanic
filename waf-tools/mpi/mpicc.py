#! /usr/bin/env python
# encoding: utf-8

import os,sys
import Configure,Options,Utils
import ccroot,ar
from Configure import conftest
def find_mpicc(conf):
	cc=conf.find_program(['mpicc'],var='CC',mandatory=True)
	cc=conf.cmd_to_list(cc)
	ccroot.get_cc_version(conf,cc,gcc=True)
	conf.env.CC_NAME='mpicc'
	conf.env.CC=cc
def mpicc_common_flags(conf):
	v=conf.env
	v['CCFLAGS_DEBUG']=['-g']
	v['CCFLAGS_RELEASE']=['-O2']
	v['CC_SRC_F']=''
	v['CC_TGT_F']=['-c','-o','']
	v['CPPPATH_ST']='-I%s'
	if not v['LINK_CC']:v['LINK_CC']=v['CC']
	v['CCLNK_SRC_F']=''
	v['CCLNK_TGT_F']=['-o','']
	v['LIB_ST']='-l%s'
	v['LIBPATH_ST']='-L%s'
	v['STATICLIB_ST']='-l%s'
	v['STATICLIBPATH_ST']='-L%s'
	v['RPATH_ST']='-Wl,-rpath,%s'
	v['CCDEFINES_ST']='-D%s'
	v['SONAME_ST']='-Wl,-h,%s'
	v['SHLIB_MARKER']='-Wl,-Bdynamic'
	v['STATICLIB_MARKER']='-Wl,-Bstatic'
	v['FULLSTATIC_MARKER']='-static'
	v['program_PATTERN']='%s'
	v['shlib_CCFLAGS']=['-fPIC','-DPIC']
	v['shlib_LINKFLAGS']=['-shared']
	v['shlib_PATTERN']='lib%s.so'
	v['staticlib_LINKFLAGS']=['-Wl,-Bstatic']
	v['staticlib_PATTERN']='lib%s.a'
	v['LINKFLAGS_MACBUNDLE']=['-bundle','-undefined','dynamic_lookup']
	v['CCFLAGS_MACBUNDLE']=['-fPIC']
	v['macbundle_PATTERN']='%s.bundle'
def mpicc_modifier_win32(conf):
	v=conf.env
	v['program_PATTERN']='%s.exe'
	v['shlib_PATTERN']='%s.dll'
	v['implib_PATTERN']='lib%s.dll.a'
	v['IMPLIB_ST']='-Wl,--out-implib,%s'
	dest_arch=v['DEST_CPU']
	if dest_arch=='x86':
		v['shlib_CCFLAGS']=['-DPIC']
	v.append_value('shlib_CCFLAGS','-DDLL_EXPORT')
	v.append_value('LINKFLAGS','-Wl,--enable-auto-import')
def mpicc_modifier_cygwin(conf):
	mpicc_modifier_win32(conf)
	v=conf.env
	v['shlib_PATTERN']='cyg%s.dll'
	v.append_value('shlib_LINKFLAGS','-Wl,--enable-auto-image-base')
def mpicc_modifier_darwin(conf):
	v=conf.env
	v['shlib_CCFLAGS']=['-fPIC','-compatibility_version','1','-current_version','1']
	v['shlib_LINKFLAGS']=['-dynamiclib']
	v['shlib_PATTERN']='lib%s.dylib'
	v['staticlib_LINKFLAGS']=[]
	v['SHLIB_MARKER']=''
	v['STATICLIB_MARKER']=''
	v['SONAME_ST']=''
def mpicc_modifier_aix(conf):
	v=conf.env
	v['program_LINKFLAGS']=['-Wl,-brtl']
	v['shlib_LINKFLAGS']=['-shared','-Wl,-brtl,-bexpfull']
	v['SHLIB_MARKER']=''
def mpicc_modifier_platform(conf):
	dest_os=conf.env['DEST_OS']or Utils.unversioned_sys_platform()
	mpicc_modifier_func=globals().get('mpicc_modifier_'+dest_os)
	if mpicc_modifier_func:
		mpicc_modifier_func(conf)
def detect(conf):
	conf.find_mpicc()
	conf.find_cpp()
	conf.find_ar()
	conf.mpicc_common_flags()
	conf.mpicc_modifier_platform()
	conf.cc_load_tools()
	conf.cc_add_flags()
	conf.link_add_flags()

conftest(find_mpicc)
conftest(mpicc_common_flags)
conftest(mpicc_modifier_win32)
conftest(mpicc_modifier_cygwin)
conftest(mpicc_modifier_darwin)
conftest(mpicc_modifier_aix)
conftest(mpicc_modifier_platform)
