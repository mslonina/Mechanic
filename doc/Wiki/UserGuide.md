The Mechanic User Guide
=======================

[Table of Contents]
[Overview]
[Requirements]
[Installation]


Overview
--------

@todo


Requirements
------------

_Most of the requirements can be found in the package repository of your Linux distribution._

- GCC 4.6+ or equivalent
- MPI2 implementation (we favour [OpenMPI](http://www.open-mpi.org/))
- [HDF5 1.8+](http://www.hdfgroup.org/)
- [CMAKE and CPACK 2.8+](http://www.cmake.org/)
- [Popt](http://rpm5.org/files/popt/) (it is already present in most of Linux distros)
- Libreadconfig (see below)


Installation
------------

[All files can be found here](http://git.astri.umk.pl/projects/mechanic/files)

### System-wide installation

1. Download and install LibReadConfig helper library for Mechanic

    tar -xvvf libreadconfig-VERSION-src.tar.gz
    cd libreadconfig-VERSION-src
    mkdir build && cd build
    CC=mpicc FC=mpif90 cmake ..
    make
    sudo make install


2. Download and install the Mechanic

    tar -xvvf mechanic-VERSION-src.tar.gz
    cd mechanic-VERSION-src
    mkdir build && cd build
    CC=mpicc FC=mpif90 cmake ..
    make
    sudo make install


3. Download and install the ArnoldWeb sample module

   tar -xvvf mechanic_module_arnold-VERSION-Source.tar.gz
   cd mechanic_module_arnold-VERSION-Source
   mkdir build && cd build
   CC=mpicc cmake ..
   make
   sudo make install


### Local installation

In case of local installation, you should basically follow system-wide installation and change the CMake install path with:

    -DCMAKE_INSTALL_PREFIX:PATH=/your/path


- Bash users, i.e.:

    CC=mpicc cmake .. -DCMAKE_INSTALL_PREFIX:PATH=/home/bob/opt


- Tcsh users, i.e:

    setenv CC mpicc && cmake .. -DCMAKE_INSTALL_PREFIX:PATH=/home/bob/opt

After the installation you should adjust the `PATH` variable and shared library search path accordingly, i.e.:

- Bash users, i.e.:

    export PATH=/home/bob/opt/bin:$PATH
    export LD_LIBRARY_PATH=/home/bob/opt/lib:$LD_LIBRARY_PATH


- Tcsh users, i.e.:

    setenv PATH /home/bob/opt/bin:$PATH
    setenv LD_LIBRARY_PATH /home/bob/opt/lib:$LD_LIBRARY_PATH


_Note: Mac users will adjust `DYLD_LIBRARY_PATH`_.


