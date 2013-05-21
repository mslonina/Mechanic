Mechanic 2.x installation guide
===============================

Requirements
------------

Mechanic should run on any Unix-like platforms (Linux and OS X are actively
maintained), if the following requirements are met:

- GCC >= 4.2 (CLang, Intel)
- CMake >= 2.8
- OpenMPI >= 1.6 (or other MPI2 implementation)
- HDF5 >= 1.8
- Popt library >= 1.14

All of the above packages should be available in the package manager of your distribution.
OS X users may use [homebrew](http://mxcl.github.com/homebrew/) or other package manager,
however, we recommend the [Gentoo Prefix](http://www.gentoo.org/proj/en/gentoo-alt/prefix/)
(see instructions below).

Notes:
- GCC 4.6+ is required for the support of the Fortran2003+ `iso_c_binding` (C
interoperability)
- CUDA 5.0 supports up to GCC 4.6
- GCC support is OS X is provided by the command line tools of the XCode distribution

Manual Compilation
------------------

    tar -xvvf mechanic-2.3.0.tar.gz
    cd mechanic-2.3.0
    mkdir build
    cd build
    CC=mpicc cmake ..
    make
    make install

By default, CMake installs to `/usr/local`. You can change the installation path by setting
    
    -DCMAKE_INSTALL_PREFIX:PATH=/your/custom/path

To run unit tests try:

    make test

Tests are available as examples in the `examples/c` directory.

Using the installation script
-----------------------------

We provide very simple install shell script to help in installation of requirements (such as
HDF, MPI). By default, the script will install all dependencies and the
Mechanic in the `mechanic-opt` directory under the current working dir. You may edit the
file and change some default settings, such as installation path or compilers.

The script requires wget/curl and cmake to be installed on your system.

First, export `PATH` and `LD_LIBRARY_PATH` to point to our new Mechanic environment
(Mac OS X users need to adjust `DYLD_LIBRARY_PATH` insted of `LD_LIBRARY_PATH`):

    export PATH=/path/to/mechanic-opt/bin:$PATH
    export LD_LIBRARY_PATH=/path/to/mechanic-opt/lib:$LD_LIBRARY_PATH

Before running the script, please edit following variables to point to your compiler:

    # CC=icc
    # CXX=icpc
    # FC=ifort
    # F77=ifort

Running the script:

    chmod +x install_mechanic.sh
    ./install_mechanic.sh

After the successfull installation, for an everyday usage, you may adjust your shell
variables to point to the Mechanic environment, i.e.

    # for Bash users, edit .bashrc file:
    export PATH=/path/to/mechanic-opt/bin:$PATH
    export LD_LIBRARY_PATH=/path/to/mechanic-opt/lib:$LD_LIBRARY_PATH


Gentoo users
------------

There is a `mechanic-overlay` prepared for Gentoo/Gentoo Prefix users, see
[mechanic overlay](http://github.com/mslonina/mechanic-overlay).
This is the preferred way of installing the Mechanic and its dependencies.
After you install the overlay, you may use:

    emerge sci-misc/mechanic

Python toolkit
--------------

To install Python toolkit, based on matplotlib and h5py, use the following:

    CC=mpicc cmake .. -DBUILD_PYTHON_TOOLKIT:BOOL=ON


RNGS library
------------

The RNGS library is built by default, to turn it off use:

    CC=mpicc cmake .. -DBUILD_VENDOR_RNGS:BOOL=OFF

