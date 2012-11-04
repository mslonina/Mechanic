Mechanic short changelog
========================

2.2
---

- Support for all basic (native) datatypes
- Multidimensional datasets (min. rank 2 up to max rank H5S_MAX_RANK)
- Support for 3D task board, and additional task information right at the board
- New memory handling, much more flexible and efficient
- New advanced hooks: NodePrepare(), NodeProcess(), LoopPrepare() and LoopProcess()
- Several bug fixes and minor improvements
- Documentation updates

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
