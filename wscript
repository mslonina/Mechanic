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

#LD_LIBRARY_PATH = string.split(os.environ.get("LD_LIBRARY_PATH"),':')
#INCLUDE_PATH = string.split(os.environ.get("C_INCLUDE_PATH"),':')

srcdir = '.'
blddir = 'build'

stdlibs = ['stdio.h', 'dlfcn.h', 'locale.h', 'memory.h', 'stdlib.h', 'stdint.h', 'inttypes.h', 
           'strings.h', 'string.h', 'sys/stat.h', 'sys/types.h', 'unistd.h']

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
#define HAVE_HDF5_SUPPORT 1
#include "libreadconfig.h"
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

# [Helper] Check for standard headers/functions
def _check_std_headers(conf, stdlibs):

  for i in stdlibs:
    j = string.split(i,'.h')
    l = string.split(j[0], '/')
    if len(l) > 1:
      j[0] = l[0] + l[1] 

    k = 'HAVE_' + j[0].upper() + '_H'
    conf.check_cc(header_name=i, define_name=k, mandatory=True)

  conf.check_cc(type_name='ssize_t', header_name='sys/types.h')
  conf.check_cc(type_name='int', header_name='sys/types.h')
  conf.check_cc(type_name='char', header_name='sys/types.h')
  conf.check_cc(type_name='double', header_name='sys/types.h')
  conf.check_cc(function_name='printf', header_name='stdio.h', mandatory=True)
  conf.check_cc(function_name='memmove', header_name='memory.h', mandatory=True)
  conf.check_cc(function_name='strcspn', header_name='string.h', mandatory=True)
  conf.check_cc(function_name='strspn', header_name='string.h', mandatory=True)
  conf.check_cc(function_name='strstr', header_name='string.h', mandatory=True)
  conf.check_cc(function_name='strtol', header_name='stdlib.h', mandatory=True)
  conf.check_cc(function_name='strtok', header_name='string.h', mandatory=True)

def _check_popt(conf):
  try:
    conf.check_cc(lib="popt", uselib_store="POPT", mandatory=True, msg="Looking for POPT library")
  except:
    print "POPT library was not found on your system"

  try:
    conf.check_cc(header_name='popt.h', uselib="POPT", mandatory=True)
  except:
    print "POPT header was not found on your system :("


# [Helper] Check for the HDF5 library
def _check_hdf5(conf):
  try:
    conf.check_cc(lib='hdf5', uselib_store="HDF5", mandatory=True, msg="Looking for HDF5 library")
  except:
    print "HDF5 library was not found on your system :("

  try:
    conf.check_cc(header_name='hdf5.h', uselib="HDF5", mandatory=True)
  except:
    print "HDF5 header was not found on your system :("

  try:
    conf.check_cc(fragment=hdf5_test_code, 
                  execute=True, 
                  uselib="HDF5", 
                  define_ret=True, 
									define_name="HAVE_HDF5_SUPPORT",
                  mandatory=True, 
                  msg="Checking if HDF5 is usable")
  except:
    print "Cannot run HDF5 testprogram :("

# [Helper] Check for the HDF5 library
def _check_lrc(conf):
  try:
    conf.check_cc(lib='readconfig', 
                  mandatory=True, 
                  uselib_store="LRC",
                  msg="Looking for LRC library");
  except:
    print "LRC library was not found on your system :("

  try:
    conf.check_cc(header_name='libreadconfig.h', 
                  mandatory=True)
  except:
    print "LRC header was not found on your system :("

  try:
    conf.check_cc(fragment=lrc_hdf_test_code, 
                  execute=True, 
                  uselib="HDF5 LRC",
                  define_ret=True, 
                  mandatory=True, 
                  msg="Checking if LRC is usable")
  except:
    print "Cannot run LRC testprogram :("



# [Helper] Check for MPI
def _check_mpi(conf):
  conf.find_program('mpicc', mandatory=False)

# [Helper] Yes/No at the summary
def _check_defined(conf, define):
	if conf.is_defined(define):
		return "Yes"
	else:
		return "No"

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
  global stdlibs
  global hdf5_test_code
  conf.check_tool('compiler_cc')
  _check_mpi(conf)
  _check_std_headers(conf, stdlibs)
  _check_popt(conf)
  _check_hdf5(conf)
  _check_lrc(conf)
	
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
  conf.env['CCFLAGS'] += ['-Wall']
  conf.env['CPPFLAGS'] += ['-DHAVE_CONFIG_H']
  conf.env['CPPFLAGS'] += ['-I../build/default']
  conf.env['CPPFLAGS'] += ['-Icore -I.']
  conf.env['CPPFLAGS'] += ['-I.']

  # Write config.h
  conf.write_config_header('config.h')

#
# BUILD
#
def build(bld):
  bld.add_subdirs('src')
  bld.install_files('${PREFIX}/include', 'src/core/mechanic.h')
  pass

