
Requirements
------------

Mechanic should run on Unix-like platforms (Linux and MacOSX are actively
maintained), if following requirements are met:

- gcc >= 4.6 or ifort >= 11.0
- CMake >= 2.8
- MPI2 implementation (optional: Fortran support) (we favour OpenMPI)
- HDF5 >= 1.8
- Popt library >= 1.14
- Libreadconfig >= 0.12.4 with HDF5 support (see http://git.astri.umk.pl/projects/lrc)

Compilation
-----------

    tar -xvvf mechanic-0.12.7-src.tar.gz
    cd mechanic-0.12.7
    mkdir build
    cd build
    CC=mpicc FC=mpif90 cmake ..
    make
    make install

By default, Cmake installs to /usr/local. You can change this by setting
    
    -DCMAKE_INSTALL_PREFIX:PATH=/your/custom/path

Fortran support
---------------

Mechanic supports direct Fortran2003+ bindings (using the iso_c_binding module).
This make it possible to write entire module in Fortran. To enable direct bindings run

    CC=mpicc cmake .. -DBUILD_FORTRAN:BOOL=ON

Otherwise, the C-module must connect with Fortran code in a compiler-specific way.

Additional modules
------------------

All but core module have been moved to separate project.
See http://git.astri.umk.pl/projects/mechanic_modules

Gentoo users
------------

There is a mechanic-overlay prepared for Gentoo/Gentoo Prefix users, see
http://github.com/mslonina/mechanic-overlay. After you install the overlay, you may use:

    emerge =mechanic-0.12.7
