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
CC=icc
CXX=icpc
F77=ifort
FC=ifort
VERSION=2.0.0_rc5

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
curl http://www.open-mpi.org/software/ompi/v1.5/downloads/openmpi-1.5.5.tar.bz2 -o openmpi-1.5.5.tar.bz2
curl http://www.hdfgroup.org/ftp/HDF5/current/src/hdf5-1.8.8.tar.bz2 -o hdf5-1.8.8.tar.bz2

# Download the Mechanic environment
curl -L https://github.com/mslonina/LibReadConfig/tarball/0.12.4 -o libreadconfig-0.12.4.tar.gz
curl -L https://github.com/mslonina/Mechanic/tarball/${VERSION} -o mechanic-${VERSION}.tar.gz

export CC=${CC}
export CXX=${CXX}
export FC=${FC}
export F77=${F77}

# Install Openmpi
tar -xvvf openmpi-1.5.5.tar.bz2
cd openmpi-1.5.5
./configure --prefix=$mdir
make $opts && make install

# Install HDF5
cd ..
tar -xvvf hdf5-1.8.8.tar.bz2
cd hdf5-1.8.8
./configure --prefix=$mdir
#./configure CC=icc CXX=icpc F77=ifort FC=ifort --enable-fortran --prefix=$mdir
make $opts && make install

# Install LRC
cd ..
mkdir libreadconfig-0.12.4
tar -zxvf libreadconfig-0.12.4.tar.gz -C libreadconfig-0.12.4 --strip-components=1
cd libreadconfig-0.12.4/
mkdir build
cd build

CC=mpicc cmake .. -DBUILD_HDF5:BOOL=ON -DCMAKE_INSTALL_PREFIX:PATH="$mdir"
make $opts && make install

# Install Mechanic
cd ../../
mkdir mechanic-${VERSION}
tar -zxvf mechanic-${VERSION}.tar.gz -C mechanic-${VERSION} --strip-components=1
cd mechanic-${VERSION}/
mkdir build
cd build

CC=mpicc cmake .. -DCMAKE_INSTALL_PREFIX:PATH="$mdir"
make $opts && make install

