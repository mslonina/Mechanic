Mechanic  2.x roadmap
=====================

2.2.x
-----

1. New memory allocator
  [DONE] Move memory buffers: s->data => s->memory (keep double type and rank 2)
  [DONE] Move datatypes: double => generic (map HDF5/MPI/LRC atomic datatypes, keep rank 2)
  - Allow rank > 2 datasets

2.3.x
-----

1. Allow usage of storage.attr (automatic storage of attributes for given datasets)

2.4.x
-----

1. Setup subsystem updates
  - Save configuration as attributes (for better efficiency)

