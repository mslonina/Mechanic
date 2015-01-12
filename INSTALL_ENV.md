Installing the Mechanic environment
===================================

Installing the Mechanic environment might be a straightforward task, especially on an
outdated cluster, without administrative access. Mechanic depends on recent versions of
computing software, thus, you may find installation notes below to be helpful. We assume
here, the installation will be performed locally, in a user directory. Since for many scientific
applications Fortran support is required, we enable here full Fortran2003+ support (the
`iso_c_binding` must be provided by the compiler).

Prepare the environment
-----------------------

    mkdir ~/mechanic-env
    mkdir ~/mechanic-env/src
    cd ~/mechanic-env/src

    export PATH=~/mechanic-env/bin:$PATH
    export LD_LIBRARY_PATH=~/mechanic-env/lib:$LD_LIBRARY_PATH

Compiler
--------

Mechanic should compile on any recent compiler (usually GCC4.6+ family will be used, which
is critical for proper Fortran support), however, some steps are neccessary for successfull compilation.

**It is critical to build the environment with the same compiler family**

### Using GCC compiler family
    
    export CC=gcc
    export CXX=g++
    export FC=gfortran
    export F77=gfortran

### Using the Intel compiler family

    export CC=icc
    export CXX=icpc
    export FC=ifort
    export F77=ifort

CMake 2.8+
----------

Download CMake from http://www.cmake.org/cmake/resources/software.html

    cd cmake
    ./bootstrap --prefix=~/mechanic-env
    gmake
    make install

HDF5 1.8+
---------

Download the HDF5 library from http://www.hdfgroup.org/ftp/HDF5/current/src/

    cd hdf5
    ./configure --enable-fortran --prefix=~/mechanic-env
    make
    make install

OpenMPI 1.8+
------------

Download the OpenMPI library from http://www.open-mpi.org/software/ompi/v1.8/

    cd openmpi
    ./configure --prefix=~/mechanic-env
    make
    make install

Mechanic 2.3+
-------------

Download the Mechanic from http://github.com/mslonina/Mechanic

    cd mechanic
    mkdir build && cd build
    CC=mpicc FC=mpif90 cmake .. -DCMAKE_INSTALL_PREFIX:PATH=~/mechanic-env

If cmake fails with CMAKE_ROOT not found, try:
    
    CC=mpicc FC=mpif90 ~/mechanic-env/bin/cmake .. -DCMAKE_INSTALL_PREFIX:PATH=~/mechanic-env

