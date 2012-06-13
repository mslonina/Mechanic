Requirements
------------

Mechanic should run on Unix-like platforms (Linux and MacOSX are actively
maintained), if the following requirements are met:

- gcc >= 4.6 or ifort >= 11.0
- CMake >= 2.8
- MPI2 implementation
- HDF5 >= 1.8
- Popt library >= 1.14
- Libreadconfig with HDF5 support >= 0.12.4 (see http://git.astri.umk.pl/projects/lrc)

Compilation
-----------

    tar -xvvf mechanic-2.0.0-src.tar.gz
    cd mechanic-2.0.0
    mkdir build
    cd build
    CC=mpicc cmake ..
    make
    make install

By default, Cmake installs to /usr/local. You can change this by setting
    
    -DCMAKE_INSTALL_PREFIX:PATH=/your/custom/path

Using the installation script
-----------------------------

We provide simple install shell script to help in installation of requirements (such as
HDF, MPI, Libreadconfig). By default, the script will install all dependencies and the
Mechanic in the `mechanic-opt` directory under the current working dir. You may edit the
file and change some default settings, such as installation path or compilers.

The script requires curl and cmake to be installed on your system.

    # edit the install_mechanic.sh
    # change CC, CXX, FC, F77 to point to your compilers, i.e.
    # CC=icc
    # CXX=icpc
    # FC=ifort
    # F77=ifort

    # run the script
    chmod +x install_mechanic.sh
    ./install_mechanic.sh

After the successfull installation you should adjust `LD_LIBRARY_PATH` and `PATH` variables to
point to the Mechanic environment, i.e.

    # for Bash users, edit .bashrc file:
    export PATH=/path/to/mechanic-opt/bin:$PATH
    export LD_LIBRARY_PATH=/path/to/mechanic-opt/lib:$LD_LIBRARY_PATH

Note: MAC users need to adjust `DYLD_LIBRARY_PATH`

Gentoo users
------------

There is a mechanic-overlay prepared for Gentoo/Gentoo prefix users, see
http://github.com/mslonina/mechanic-overlay. This is the preferred way of installing the
Mechanic and its dependencies. After you install the overlay, you may use:

    emerge =mechanic-2.0.0_rc5

Additional modules
------------------

All but core module have been moved to separate project.
See http://git.astri.umk.pl/projects/mechanic_modules
