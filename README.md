Mechanic
--------

The Mechanic is a task management system and host software framework developed to help in
massive numerical simulations. It provides powerful and flexible user API with unified
data storage and management. It relies on the core-module approach, which allows to
separate the numerical problem from the common programming tasks, such as setup, storage,
task management, splitting the workload, checkpointing etc. From this point of view it
allows the user to focus on the numerical part of the scientific problem only, without
digging into MPI or advanced data storage. Since the user API is written in plain C, it
allows to easily adapt any code developed with a C-interoperable programming language,
such as C++, Fortran2003+, OpenCL, CUDA etc.

The core code is based on the MPI Task Farm model and the HDF5 data storage specification.
It may be installed system-wide and become a helper tools for users, who need to perform a
large number of computations. The code has been tested on large CPU-clusters, as well as
desktop computers and works equally well (the Linux and MAC OS X operating systems are
actively maintained).

Mechanic is BSD-licensed. The source code package comes with few example
modules and is freely available at http://git.astri.umk.pl/projects/mechanic

Key features
------------

- **The numerical part of the code is fully separated from the setup and storage phase.** You may
  use the user API to adjust the storage layout and configuration options
- **Storage of data and configuration.** The core code takes care on the data storage and
  configuration
- **The pool-based simulations.** Each pool may have different number of task to compute
  (the task loop follows the MPI Task Farm model), and data from all pools may be used at
  the every stage of the simulation
- **MPI non-blocking communication**
- **HDF5 data storage layout**
- **Automatic backup of data files and restart mode**
- **Configuration command line.** All configuration options defined through API are
  automatically available in the command line
- **Different storage modes** which allows to design the storage layout that fits best
  the user needs (i.e. for processing with Gnuplot or Matplotlib)
- **Linux and MAC OS X** supported

Example usage
-------------

- **Efficient creating of dynamical maps.** Each pixel of the map is mapped into
  standalone numerical task
- **Genetic algorithms.** Each pool of tasks may be treated as a generation in the
  language of GAs
- **Data processing.** Think about processing huge number of astronomical observations

Differences to other task management systems
--------------------------------------------

The Mechanic differs significantly from other task management systems, such as Condor or
Workqueue, in terms of user API: the code does not use the executable of user's serial
code. Instead, the user's code should be rewritten within the provided API. In such a way, we
focus only on the numerical part of the task, and not its setup or storage. The HDF5
storage layer provides unified way to access the data by a number of different
applications (not to mention C/Fortran codes only, but also Python software)

Short history of Mechanic
-------------------------

The idea of creating the Mechanic came from the problem of efficient computing of dynamical maps of
planetary systems. Such maps consist of many many independent numerical simulations which
may take a long time (from seconds to weeks). Efficient splitting of such workload was
neccessary to determine the global dynamics of exo-planets. The MPI-software-skeleton-kind idea
that came once to K. Goździewski mind was expanded and developed by his PhD student, M.
Słonina. The very first branch of the Mechanic, proof-of-concept 0.12 was successfully used on several
clusters and became a good starting point for the full featured software.

Publications
------------

- Slonina M., Gozdziewski K., Migaszewski C., 2012arXiv1202.6513S
- Migaszewski C., Slonina M., Gozdziewski K., 2012arXiv1205.0822M
- Gozdziewski K. et al, 2012arXiv1205.4164G
- Slonina M., Gozdziewski K., Migaszewski C., Rozenkiewicz A., 2012arXiv1205.1341S
- Slonina M., Gozdziewski K., Migaszewski C., Astrophysics Source Code Library, record
ascl:1205.001

Posters
-------

- Slonina M., Gozdziewski K., Migaszewski C., Simtech2011 (Stuttgart, June 2011)
- Slonina M., Gozdziewski K., Migaszewski C., Orbital Couples: "Pas de Deux" in the Solar
  System and the Milky Way(Paris, October 2011)

Acknowledgments
---------------

This project is supported by the Polish Ministry of Science and Higher Education through
the grant N/N203/402739. This work is conducted within the POWIEW project of the European
Regional Development Fund in Innovative Economy Programme POIG.02.03.00-00-018/08.
