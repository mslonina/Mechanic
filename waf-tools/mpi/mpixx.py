#! /usr/bin/env python
# encoding: utf-8

import os,sys
import Configure,Options,Utils
import ccroot,ar
from Configure import conftest
def find_mpicxx(conf):
	cxx=conf.find_program(['mpicxx','mpic++'],var='CXX',mandatory=True)
	cxx=conf.cmd_to_list(cxx)
	ccroot.get_cc_version(conf,cxx,gcc=True)
	conf.env.CXX_NAME='mpicc'
	conf.env.CXX=cxx
def mpicxx_common_flags(conf):
	v=conf.env
	v['CXXFLAGS_DEBUG']=['-g']
	v['CXXFLAGS_RELEASE']=['-O2']
	v['CXX_SRC_F']=''
	v['CXX_TGT_F']=['-c','-o','']
	v['CPPPATH_ST']='-I%s'
	if not v['LINK_CXX']:v['LINK_CXX']=v['CXX']
	v['CXXLNK_SRC_F']=''
	v['CXXLNK_TGT_F']=['-o','']
	v['LIB_ST']='-l%s'
	v['LIBPATH_ST']='-L%s'
	v['STATICLIB_ST']='-l%s'
	v['STATICLIBPATH_ST']='-L%s'
	v['RPATH_ST']='-Wl,-rpath,%s'
	v['CXXDEFINES_ST']='-D%s'
	v['SONAME_ST']='-Wl,-h,%s'
	v['SHLIB_MARKER']='-Wl,-Bdynamic'
	v['STATICLIB_MARKER']='-Wl,-Bstatic'
	v['FULLSTATIC_MARKER']='-static'
	v['program_PATTERN']='%s'
	v['shlib_CXXFLAGS']=['-fPIC','-DPIC']
	v['shlib_LINKFLAGS']=['-shared']
	v['shlib_PATTERN']='lib%s.so'
	v['staticlib_LINKFLAGS']=['-Wl,-Bstatic']
	v['staticlib_PATTERN']='lib%s.a'
	v['LINKFLAGS_MACBUNDLE']=['-bundle','-undefined','dynamic_lookup']
	v['CCFLAGS_MACBUNDLE']=['-fPIC']
	v['macbundle_PATTERN']='%s.bundle'
def mpicxx_modifier_win32(conf):
	v=conf.env
	v['program_PATTERN']='%s.exe'
	v['shlib_PATTERN']='%s.dll'
	v['implib_PATTERN']='lib%s.dll.a'
	v['IMPLIB_ST']='-Wl,--out-implib,%s'
	dest_arch=v['DEST_CPU']
	if dest_arch=='x86':
		v['shlib_CXXFLAGS']=['-DPIC']
	v.append_value('shlib_CXXFLAGS','-DDLL_EXPORT')
	v.append_value('LINKFLAGS','-Wl,--enable-auto-import')
def mpicxx_modifier_cygwin(conf):
	mpicxx_modifier_win32(conf)
	v=conf.env
	v['shlib_PATTERN']='cyg%s.dll'
	v.append_value('shlib_LINKFLAGS','-Wl,--enable-auto-image-base')
def mpicxx_modifier_darwin(conf):
	v=conf.env
	v['shlib_CXXFLAGS']=['-fPIC','-compatibility_version','1','-current_version','1']
	v['shlib_LINKFLAGS']=['-dynamiclib']
	v['shlib_PATTERN']='lib%s.dylib'
	v['staticlib_LINKFLAGS']=[]
	v['SHLIB_MARKER']=''
	v['STATICLIB_MARKER']=''
	v['SONAME_ST']=''
def mpicxx_modifier_aix(conf):
	v=conf.env
	v['program_LINKFLAGS']=['-Wl,-brtl']
	v['shlib_LINKFLAGS']=['-shared','-Wl,-brtl,-bexpfull']
	v['SHLIB_MARKER']=''
def mpicxx_modifier_platform(conf):
	dest_os=conf.env['DEST_OS']or Utils.unversioned_sys_platform()
	mpicxx_modifier_func=globals().get('mpicxx_modifier_'+dest_os)
	if mpicxx_modifier_func:
		mpicxx_modifier_func(conf)
def detect(conf):
	conf.find_mpicxx()
	conf.find_cpp()
	conf.find_ar()
	conf.mpicxx_common_flags()
	conf.mpicxx_modifier_platform()
	conf.cxx_load_tools()
	conf.cxx_add_flags()

conftest(find_mpicxx)
conftest(mpicxx_common_flags)
conftest(mpicxx_modifier_win32)
conftest(mpicxx_modifier_cygwin)
conftest(mpicxx_modifier_darwin)
conftest(mpicxx_modifier_aix)
conftest(mpicxx_modifier_platform)
