#! /usr/bin/env python
# encoding: utf-8

import os,sys
import Configure,Options,Utils
import ccroot,ar
from Configure import conftest
def find_nvcc(conf):
	cc=conf.find_program(['nvcc'],var='NVCC',mandatory=True)
	cc=conf.cmd_to_list(cc)
	#ccroot.get_cc_version(conf,cc,gcc=True)
	conf.env['NVCC_NAME']='nvcc'
	conf.env['NVCC']=cc
def nvcc_common_flags(conf):
	v=conf.env
	v['NVCCFLAGS_DEBUG']=['-g']
	v['NVCCFLAGS_RELEASE']=['-O2']
	v['NVCC_SRC_F']=''
	v['NVCC_TGT_F']=['-c','-o','']
	v['CPPPATH_ST']='-I%s'
	if not v['LINK_NVCC']:v['LINK_NVCC']=v['NVCC']
	v['NVCCLNK_SRC_F']=''
	v['NVCCLNK_TGT_F']=['-o','']
	v['LIB_ST']='-l%s'
	v['LIBPATH_ST']='-L%s'
	v['STATICLIB_ST']='-l%s'
	v['STATICLIBPATH_ST']='-L%s'
	v['RPATH_ST']='-Wl,-rpath,%s'
	v['NVCCDEFINES_ST']='-D%s'
	v['SONAME_ST']='-Wl,-h,%s'
	v['SHLIB_MARKER']='-Wl,-Bdynamic'
	v['STATICLIB_MARKER']='-Wl,-Bstatic'
	v['FULLSTATIC_MARKER']='-static'
	v['program_PATTERN']='%s'
	v['shlib_NVCCFLAGS']=['-fPIC','-DPIC']
	v['shlib_LINKFLAGS']=['-shared']
	v['shlib_PATTERN']='lib%s.so'
	v['staticlib_LINKFLAGS']=['-Wl,-Bstatic']
	v['staticlib_PATTERN']='lib%s.a'
	v['LINKFLAGS_MACBUNDLE']=['-bundle','-undefined','dynamic_lookup']
	v['NVCCFLAGS_MACBUNDLE']=['-fPIC']
	v['macbundle_PATTERN']='%s.bundle'
def nvcc_modifier_win32(conf):
	v=conf.env
	v['program_PATTERN']='%s.exe'
	v['shlib_PATTERN']='%s.dll'
	v['implib_PATTERN']='lib%s.dll.a'
	v['IMPLIB_ST']='-Wl,--out-implib,%s'
	dest_arch=v['DEST_CPU']
	if dest_arch=='x86':
		v['shlib_NVCCFLAGS']=['-DPIC']
	v.append_value('shlib_NVCCFLAGS','-DDLL_EXPORT')
	v.append_value('LINKFLAGS','-Wl,--enable-auto-import')
def nvcc_modifier_cygwin(conf):
	nvcc_modifier_win32(conf)
	v=conf.env
	v['shlib_PATTERN']='cyg%s.dll'
	v.append_value('shlib_LINKFLAGS','-Wl,--enable-auto-image-base')
def nvcc_modifier_darwin(conf):
	v=conf.env
	v['shlib_NVCCFLAGS']=['-fPIC','-compatibility_version','1','-current_version','1']
	v['shlib_LINKFLAGS']=['-dynamiclib']
	v['shlib_PATTERN']='lib%s.dylib'
	v['staticlib_LINKFLAGS']=[]
	v['SHLIB_MARKER']=''
	v['STATICLIB_MARKER']=''
	v['SONAME_ST']=''
def nvcc_modifier_aix(conf):
	v=conf.env
	v['program_LINKFLAGS']=['-Wl,-brtl']
	v['shlib_LINKFLAGS']=['-shared','-Wl,-brtl,-bexpfull']
	v['SHLIB_MARKER']=''
def nvcc_modifier_platform(conf):
	dest_os=conf.env['DEST_OS']or Utils.unversioned_sys_platform()
	nvcc_modifier_func=globals().get('nvcc_modifier_'+dest_os)
	if nvcc_modifier_func:
		nvcc_modifier_func(conf)
def detect(conf):
	conf.find_nvcc()
	conf.find_cpp()
	conf.find_ar()
	conf.nvcc_common_flags()
	conf.nvcc_modifier_platform()
	conf.cc_load_tools()
	conf.cc_add_flags()
	conf.link_add_flags()

conftest(find_nvcc)
conftest(nvcc_common_flags)
conftest(nvcc_modifier_win32)
conftest(nvcc_modifier_cygwin)
conftest(nvcc_modifier_darwin)
conftest(nvcc_modifier_aix)
conftest(nvcc_modifier_platform)
