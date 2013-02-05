Mechanic short changelog
========================

2.3
---

- Task snapshots - possibility to save current state of the task to the checkpoint buffer,
  and continue task processing (instead of storing the task state after the task has
  been processed)
- ICE check. If the master node founds mechanic.ice file, it will abort the run
- Support for numeric and string attributes

### Configuration

- Runtime configuration is stored as attributes attached to the task board. No more `/Config` dataset
- The `setup` object has been removed from the API. Runtime configuration may be now modified
  per task pool through the `MReadOption` and `MWriteOption` macros
- New core configuration options: x/y/z-axis element, x/y/z-label, as well as common
  module-like options such as: debug, dense etc.

### Hooks

- New hooks: Send() and Receive(), we are invoked after MPI_Send and MPI_Receive
  respectively, on each node

### Loadable runtime mode support

- Now, the user may create additional library with the runtime mode (this is an advanced
  case of using the Mechanic, and we suggest to look into the core taskfarm mode). The
  library prefix is `libmechanic_mode_${LIBRARY}` and is should contain `Master()` and
  `Worker()` functions. No core fallback is provided. This fully fits the original idea of
  the Mechanic. You can switch to different runtime mode with `--mode` option.

### Other

- Initial work for python postprocessing pipeline
- Mechanic now ships with the RNGS random number library 
- Documentation updates
- Bug fixes

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

