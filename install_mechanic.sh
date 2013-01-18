#!/bin/bash
#
# Helper script to install the Mechanic environment,
# including installation of OpenMPI and HDF5
#
# Requirements: wget/curl, cmake
#

# Your setting goes here
dir=`pwd`
mdir=${dir}/mechanic-opt

# Your compilers
CC=gcc #icc
CXX=g++ #icpc
F77=gfortran #ifort
FC=gfortran #ifort

# software versions
MECHANIC=2.2.6
HDF=1.8.10
MPI=1.6.3

# make opts
opts=-j4

#
# Do not touch unless you know what're doing ;)
#

USE_WGET=1
USE_CURL=1

DWN="curl -L"
DWNO=-o

command -v curl || USE_CURL=0
if [[ ${USE_CURL} -eq 0 ]]; then
  command -v wget || USE_WGET=0
  if [[ ${USE_WGET} -eq 0 ]]; then
    echo "Neither curl or wget found in your system. Aborting"
    exit
  fi

  if [[ ${USE_WGET} -eq 1 ]]; then
    echo "Using wget"
    DWN="wget --no-check-certificate"
    DWNO=-O
  fi
fi


# Create the mechanic-opt dir for local installation
mkdir -p ${mdir}

# Adjust the neccessary paths
export LD_LIBRARY_PATH=${mdir}/lib:${LD_LIBRARY_PATH}
export PATH=${mdir}/bin:${PATH}

cd $mdir
mkdir -p src && cd src

# Download the OpenMPI and HDF5
${DWN} http://www.open-mpi.org/software/ompi/v1.6/downloads/openmpi-${MPI}.tar.bz2 ${DWNO} openmpi-${MPI}.tar.bz2
${DWN} http://www.hdfgroup.org/ftp/HDF5/current/src/hdf5-${HDF}.tar.bz2 ${DWNO} hdf5-${HDF}.tar.bz2

# Download the Mechanic environment
${DWN} https://github.com/mslonina/Mechanic/archive/${MECHANIC}.tar.gz ${DWNO} mechanic-${MECHANIC}.tar.gz

export CC=${CC}
export CXX=${CXX}
export FC=${FC}
export F77=${F77}

# Install Openmpi
tar -xvvf openmpi-${MPI}.tar.bz2
cd openmpi-${MPI}
./configure --prefix=$mdir
make $opts && make install

# Install HDF5
cd ..
tar -xvvf hdf5-${HDF}.tar.bz2
cd hdf5-${HDF}
./configure --prefix=$mdir
#./configure CC=icc CXX=icpc F77=ifort FC=ifort --enable-fortran --prefix=$mdir
make $opts && make install

# Install Mechanic
cd ../../
mkdir mechanic-${MECHANIC}
tar -zxvf mechanic-${MECHANIC}.tar.gz -C mechanic-${MECHANIC} --strip-components=1
cd mechanic-${MECHANIC}/
mkdir build
cd build

CC=mpicc cmake .. -DCMAKE_INSTALL_PREFIX:PATH="$mdir"
make $opts && make install

