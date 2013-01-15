Mechanic short changelog
========================

2.2.1 -> 2.2.4
--------------

- ICE check. If the master node founds mechanic.ice file, it will abort the run
- New hooks: Send() and Receive()
- Configuration is stored as attributes of the task board
- Support for numeric and string attributes
- New core configuration options: x/y/z-axis element
- Documentation updates

2.2
---

The most important change is the new memory handler. It allows using different datatypes
and dimensionality for datasets. The Module API had to be changed to reflect core changes.
See the examples for the in-depth usage of the new Module API.

#### Memory

- Contiguous memory allocation for different datatypes and dimensionality
- Generic type allocation macros `MAllocate*` and corresponding functions `Allocate*` (up to rank 4)

#### Datasets and datatypes

- Support for all basic (native) datatypes
- Multidimensional datasets (min. rank 2 up to max rank `H5S_MAX_RANK`)

#### Attributes

- HDF5 attributes fully handled through the Storage API

#### Task board

- Task board is now 3D. The fourth dimension is preserved for additional task information

#### Config API

- Libreadconfig is now shipped with the core and has been moved to Config API

#### Hooks

- New advanced hooks: NodePrepare(), NodeProcess(), LoopPrepare() and LoopProcess()
- New public functions for reading and writing data:
  ReadData(), WriteData(), ReadAttr(), WriteAttr()
- Corresponding generic-type macros for reading and writing data:
  MReadData(), MWriteData(), MReadAttr(), MWriteAttr()

#### Other

- Build system improvements
- Several bug fixes and minor improvements
- Documentation updates and new examples (i.e. fortran)
- Examples are installed in share/mechanic/examples

2.1
---

- The MPI Blocking communication mode (--blocking option)
- New hooks: DatasetPrepare() and DatasetProcess(), which may be used for advanced stuff
  on HDF5 datasets (such as attributes)
- New options: xmin, xmax, ymin, ymax, xorigin, yorigin, common for many scientific modules
- Better memory management and smaller memory footprint

2.0
---

- Pool based simulations
- MPI Nonblocking communication mode
- Support for different datasets (and memory banks)
- Support for module configuration (with command line)
- Support for different storage types (i.e. for further postprocessing in Gnuplot,
  Matplotlib etc.)

