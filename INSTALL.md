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

Additional modules
------------------

All but core module have been moved to separate project.
See http://git.astri.umk.pl/projects/mechanic_modules

Gentoo users
------------

There is a mechanic-overlay prepared for Gentoo/Gentoo prefix users, see
http://github.com/mslonina/mechanic-overlay. After you install the overlay, you may use:

    emerge mechanic2
