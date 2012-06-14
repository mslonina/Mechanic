#!/bin/bash
#
# Helper script to install the Mechanic environment,
# including installation of OpenMPI and HDF5
#
# Requirements: curl, cmake
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
MECHANIC=2.0.0
HDF=1.8.9
MPI=1.5.5
LRC=0.12.4

# make opts
opts=-j4

#
# Do not touch unless you know what're doing ;)
#

# Create the mechanic-opt dir for local installation
mkdir -p ${mdir}

# Adjust the neccessary paths
export LD_LIBRARY_PATH=${mdir}/lib:${LD_LIBRARY_PATH}
export PATH=${mdir}/bin:${PATH}

cd $mdir
mkdir -p src && cd src

# Download the OpenMPI and HDF5
curl http://www.open-mpi.org/software/ompi/v1.5/downloads/openmpi-${MPI}.tar.bz2 -o openmpi-${MPI}.tar.bz2
curl http://www.hdfgroup.org/ftp/HDF5/current/src/hdf5-${HDF}.tar.bz2 -o hdf5-${HDF}.tar.bz2

# Download the Mechanic environment
curl -L https://github.com/mslonina/LibReadConfig/tarball/${LRC} -o libreadconfig-${LRC}.tar.gz
curl -L https://github.com/mslonina/Mechanic/tarball/${MECHANIC} -o mechanic-${MECHANIC}.tar.gz

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

# Install LRC
cd ..
mkdir libreadconfig-${LRC}
tar -zxvf libreadconfig-${LRC}.tar.gz -C libreadconfig-${LRC} --strip-components=1
cd libreadconfig-${LRC}/
mkdir build
cd build

CC=mpicc cmake .. -DBUILD_HDF5:BOOL=ON -DCMAKE_INSTALL_PREFIX:PATH="$mdir"
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

