Mechanic 2.x roadmap
=====================

2.2.x
-----

1. New memory allocator
  - [DONE] Move memory buffers: s->data => s->memory (keep double type and rank 2)
  - [DONE] Move datatypes: double => generic (map HDF5/MPI/LRC atomic datatypes, keep rank 2)
  - [DONE] Allow rank > 2 datasets
  - [MOVED2CONTRIB] Update Non-blocking communication mode to reflect new memory handling
2. [DONE] Task board rank > 2
3. [DONE] Allow usage of storage.attr (automatic storage of attributes for given datasets)
  - Currently to store attributes we need use DatasetPrepare/Process() hooks, with new
    attributes interface and memory handling it should be far easier

2.3.x
-----

1. Setup subsystem updates
  - Save configuration as attributes (for better efficiency)
