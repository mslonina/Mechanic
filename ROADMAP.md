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
  - [DONE] Remove Option2Int etc. from API, use new MReadOption/MWriteOption macros instead,
    operating directly on attributes of p->board
2. [DONE] Mechanic ICE check 
3. Intermediate storage
  - [DONE] Task states
  - [DONE] Intermediate checkpoint storage
  - [DONE] Support for the restart mode
  - [DONE] Temporary storage with HDF_TEMP_STORAGE
4. Loadable run modes (such as taskfarm, masteralone, collective etc.)
  - [DONE] Refactor code headers (public and private)
  - [DONE] Mode API functions: Master(), Worker()
  - [DONE] Min CPU option in Init() 

2.4.x
-----

- (HDF) Named datatypes and datasets
- (MPI) Collective communication mode
- (MPI) Genetic algorithm mode
- (MPI) Consider task dependencies
- (CONFIG) Consider switching to yaml specification for configuration and input files
- (CONFIG) Configuration sets
- (CONFIG) Minimum, maximum config values with error handling

