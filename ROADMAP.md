Mechanic 2.x roadmap
====================

2.2.x
-----

1. New memory allocator
  - [DONE] Move memory buffers: s->data => s->memory (keep double type and rank 2)
  - [DONE] Move datatypes: double => generic (map HDF5/MPI/LRC atomic datatypes, keep rank 2)
  - [DONE] Allow rank > 2 datasets
2. [DONE] Task board rank > 2
3. [DONE] Allow usage of storage.attr (automatic storage of attributes for given datasets)
  - Currently to store attributes we need use DatasetPrepare/Process() hooks, with new
    attributes interface and memory handling it should be far easier

2.3.x
-----

1. Setup subsystem updates
  - [DONE] Save configuration as attributes (for better efficiency) of the task board
  - [DONE] Restart mode read 
  - [DONE] Remove the Config dataset
2. Loadable run modes (such as taskfarm, masteralone, collective etc.)
  - Refactor code headers (public and private)
  - Mode API functions: Init(), Master(), Worker()
  - [DONE] Min CPU option in Init() 
3. [DONE] Mechanic ICE check 
4. Intermediate storage
  - [DONE] Task states
  - [DONE] Intermediate checkpoint storage
  - [DONE] Support for the restart mode
