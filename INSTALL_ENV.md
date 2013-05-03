Installing the Mechanic environment
-----------------------------------

Prepare the environment
=======================

    mkdir ~/mechanic-env
    export PATH=~/mechanic-env/bin:$PATH
    export LD_LIBRARY_PATH=~/mechanic-env/lib:$LD_LIBRARY_PATH

Compiler
========

### Using the Intel compiler

    export CC=icc
    export CXX=icpc
    export FC=ifort
    export F77=ifort

HDF5
====

Download the HDF5 library from http://www.hdfgroup.org/ftp/HDF5/current/src/

    ./configure --enable-fortran --prefix=~/mechanic-env
    make
    make install

MPI
===

Download the OpenMPI library from http://www.open-mpi.org/software/ompi/v1.6/

    ./configure --prefix=~/mechanic-env
    make
    make install

Mechanic
========

    cd build
    CC=mpicc FC=mpif90 cmake .. -DCMAKE_INSTALL_PREFIX:PATH=~/mechanic-env
