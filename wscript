#!/usr/bin/env python
# wscript for Mechanic
# Mariusz Slonina, 2010
# encoding: utf-8

import sys
import os
import Utils
import Build
import Options

import string

VERSION='0.12.0-UNSTABLE-2'
APPNAME='mechanic'
URL='http://mechanics.astri.umk.pl/projects/mechanic'
BUGS='mariusz.slonina@gmail.com'

srcdir = '.'
blddir = 'build'

# Standard headers
stdheads = ['stdio.h', 'locale.h', 'memory.h', 'stdlib.h', 'stdint.h', 'inttypes.h', 
           'strings.h', 'string.h', 'sys/stat.h', 'sys/types.h', 'unistd.h',
					 'mpi.h']

# Standard functions
stdfunc = [['printf','stdio.h',1],
					 ['memmove','memory.h',1],
           ['strcspn','string.h',1],
           ['strspn','string.h',1],
           ['strstr','string.h',1],
           ['strtol','stdlib.h',1],
           ['strtok','string.h',1],
					 ]

# Standard types
stdtypes = [['ssize_t','sys/types.h',1],
            ['int','sys/types.h',1],
            ['char','sys/types.h',1],
            ['double','sys/types.h',1]
            ]

libm_test_code = '''
#include <stdio.h>
#include "math.h"

int main(){
  int i = 2, j = 4, k = 0;
  k = pow(i,j);
  return 0;
}
'''

popt_test_code = '''
'''

dl_test_code = '''
'''

hdf5_test_code = '''
#include <stdio.h>
#include "hdf5.h"

int main()
{
  hid_t file_id, dataset_id, dataspace_id;
  hsize_t dims[2];
  herr_t status;

  H5open();

  file_id = H5Fcreate("waf.h5", H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);

  dims[0] = 2;
  dims[1] = 2;

  dataspace_id = H5Screate_simple(2, dims, NULL);
  dataset_id = H5Dcreate(file_id, "/dset", H5T_NATIVE_INT, dataspace_id, 
      H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
  
  status = H5Dclose(dataset_id);
  status = H5Sclose(dataspace_id);
  status = H5Fclose(file_id);

  H5close();

	remove("waf.h5");

  return 0;
}
'''

lrc_hdf_test_code='''
#include <stdio.h>
#include <stdlib.h>
#include "libreadconfig_hdf5.h"
#include "hdf5.h"

int main(int argc, char* argv[]){

  hid_t file;
  int opts = 1;
  int numCT = 4;

  LRC_configTypes ct[4] = {
    {"default", "char", LRC_CHAR},
    {"default", "int", LRC_INT},
    {"default", "double", LRC_DOUBLE},
    {"default", "float", LRC_FLOAT},
  };

  LRC_configNamespace cs[] = {
    {"default",{
                 {"char","aaa",LRC_CHAR},
                 {"int","44",LRC_INT},
                 {"double","12.3456",LRC_DOUBLE},
                 {"float","34.5678",LRC_FLOAT},
               },
    4}
  };
  
  file = H5Fcreate("lrc-hdf-test.h5", H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
  
  LRC_writeHdfConfig(file, cs, opts);

  H5Fclose(file);

  remove("lrc-hdf-test.h5");
	
  return 0;
}

'''
#
# [Helper] Check for standard headers/functions
#
def _check_std_headers(conf, heads, funcs, types):
  for i in heads:
    conf.check_cc(header_name=i, mandatory=True)

  for i in funcs:
    conf.check_cc(function_name=i[0], header_name=i[1], mandatory=i[2])
  
  for i in types:
    conf.check_cc(type_name=i[0], header_name=i[1], mandatory=i[2])

#
# [Helper] Check for library
#
def _check_lib(conf, l):
  var = l[0].upper()
  try:
    conf.check_cc(lib=l[0], uselib_store=var, mandatory=l[5], msg="Looking for "+l[0]+" library")
  except:
    print l[0] + " library was not found on your system"

  try:
    conf.check_cc(header_name=l[1], uselib=var, mandatory=l[5])
  except:
    print l[1] + " header was not found on your system :("
 
  if not len(l[3]) == 0:
    try:
      conf.check_cc(fragment=l[3], 
                    execute=True, 
                    uselib=var + ' ' + l[4], 
                    define_ret=True, 
                    mandatory=l[5], 
                    msg="Checking if "+l[0]+" is usable")
    except:
      print l[0] + " is not usable :("

#
# [Helper] Check for MPI
#
def _check_mpi(conf):
  conf.find_program('mpicc', var='MPI', mandatory=True)

#
# SET OPTIONS
#
def set_options(opt):
  opt.tool_options('compiler_cc')
  #opt.tool_options('compiler_mpicc')

  opt.add_option('--build-doc',
                  action = 'store_true', 
                  default = False,
                  help = 'Build documentation',
                  dest = 'builddoc'
                  )

#
# CONFIGURE
#
def configure(conf):
  global stdheads
  global stdfunc
  global stdtypes
  global hdf5_test_code
  conf.check_tool('compiler_cc')
  _check_mpi(conf)
  _check_std_headers(conf, stdheads, stdfunc, stdtypes)
  _check_lib(conf,['m','math.h','pow', libm_test_code, '', 1])
  _check_lib(conf,['hdf5','hdf5.h','H5Dopen2', hdf5_test_code, '', 1])
  _check_lib(conf,['readconfig','libreadconfig_hdf5.h','LRC_writeHdfConfig',lrc_hdf_test_code, 'HDF5', 1])
  _check_lib(conf,['dl','dlfcn.h','dlopen','','',1])
  _check_lib(conf,['popt','popt.h','poptGetContext','','',1])
	
	# Check for doxygen
  BUILDDOC="No"
  if Options.options.builddoc:
    try:
      conf.find_program('doxygen', var='DOXYGEN', mandatory=True)
      BUILDDOC="Yes"
    except:
      print "You need Doxygen installed to build documentation"
	
  # Define standard declarations
  conf.define('PACKAGE_NAME', APPNAME)
  conf.define('PACKAGE_VERSION', VERSION)
  conf.define('PACKAGE_BUGREPORT', BUGS)
  conf.define('PACKAGE_URL', URL)

  conf.env['CC'] = ['mpicc']
  conf.env['LINK_CC'] = ['mpicc']
  conf.env['CCFLAGS'] += ['-Wall']
  #conf.env['CCFLAGS'] += ['-ansi']
  #conf.env['CCFLAGS'] += ['-pedantic']
  conf.env['CCFLAGS'] += ['-fpic']
  conf.env['CCFLAGS'] += ['-Dpic']
  conf.env['CPPFLAGS'] += ['-DHAVE_CONFIG_H']
  conf.env['CPPFLAGS'] += ['-I../build/default']

  # Write config.h
  conf.write_config_header('config.h')

#
# BUILD
#
def build(bld):
  bld.add_subdirs('src')
  bld.install_files('${PREFIX}/include', 'src/core/mechanic.h')
  pass

