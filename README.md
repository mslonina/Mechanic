Mechanic
========

The Mechanic is a task management system and a host software framework developed to help 
in conducting massive numerical simulations. It provides powerful and flexible user API 
with unified data storage and management. It relies on the core-module approach, which 
allows to separate numerical problem from the common programming tasks, such as setup, 
storage, task management, splitting the workload, checkpointing etc. From this point of 
view it allows the user to focus on the numerical part of the scientific problem only, 
without digging into MPI or advanced data storage. Since the user API is written in C, 
it allows to easily adapt any code developed with a C-interoperable programming language,
such as C++, Fortran2003+, OpenCL or CUDA.

The core code is based on the _MPI Task Farm_ model and the HDF5 data storage specification.
It may be installed system-wide and become a helper tool for users, who need to perform 
a large number of serial computations. The code has been tested on large CPU-clusters, 
as well as desktop computers and works equally well (the Linux and Mac OS X operating 
systems are actively maintained).

Mechanic is BSD-licensed. The source code package comes with few example
modules and is freely available at [the project page](http://git.ca.umk.pl).

- [Installation](INSTALL.md)
- [Short introduction](Overview.md)
- [User and developer guide](UserGuide.md)

_This project is supported by the Polish Ministry of Science and Higher Education through
the grant N/N203/402739. This work is conducted within the POWIEW project of the European
Regional Development Fund in Innovative Economy Programme POIG.02.03.00-00-018/08._
