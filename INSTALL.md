Mechanic 2.x installation guide
===============================

Requirements
------------

Mechanic should run on any Unix-like platforms (Linux and MacOSX are actively
maintained), if the following requirements are met:

- gcc >= 4.6 or ifort >= 11.0
- CMake >= 2.8
- MPI2 implementation (in favour of OpenMPI)
- HDF5 >= 1.8
- Popt library >= 1.14

Manual Compilation
------------------

    tar -xvvf mechanic-2.2.0.tar.gz
    cd mechanic-2.2.0
    mkdir build
    cd build
    CC=mpicc cmake ..
    make
    make install

By default, Cmake installs to `/usr/local`. You can change the installation path by setting
    
    -DCMAKE_INSTALL_PREFIX:PATH=/your/custom/path

Using the installation script
-----------------------------

We provide very simple install shell script to help in installation of requirements (such as
HDF, MPI, Libreadconfig). By default, the script will install all dependencies and the
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

There is a mechanic-overlay prepared for Gentoo/Gentoo prefix users, see
http://github.com/mslonina/mechanic-overlay. This is the preferred way of installing the
Mechanic and its dependencies. After you install the overlay, you may use:

    emerge =mechanic-2.2_beta5


