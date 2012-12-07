Mechanic 2.x examples
=====================

List of examples
----------------

#### The `TaskProcess()`

  - A simple map or image: `mechanic_module_ex_map.c`
  - The Mandelbrot set: `mechanic_module_ex_mandelbrot.c`

#### Working with task pools (using `PoolPrepare()` and `PoolProcess()`)
  
  - Creating task pools: `mechanic_module_ex_createpool.c`
  - Reading and writing task pool data: `mechanic_module_ex_pool.c`
  - Resetting the task pool: `mechanic_module_ex_reset.c`
  - Different storage layout per task pool (basic): `mechanic_module_ex_chpoollayout.c`
  - Different storage layout per task pool (advanced): `mechanic_module_ex_chpoollayout2.c`

#### Configuration (`Init()`, `Setup()`)

  - Defining and using configuration options: `mechanic_module_ex_setup.c`

#### Datatypes and dimensionality

  - Using different datatypes, reading and writing data to the memory banks:
    `mechanic_module_ex_datatypes.c`
  - Using datasets of different datatypes and dimensionality: `mechanic_module_ex_dim.c`

#### Attributes

  - Using HDF5 attributes: `mechanic_module_ex_attr.c`

#### Advanced hooks

  - Using `Prepare()` and `Process()` hooks: `mechanic_module_ex_prepareprocess.c`
  - Using `DatasetPrepare()` and `DatasetProcess()`: `mechanic_module_ex_dataset.c`

Compilation
-----------

    mpicc -std=c99 -fPIC -Dpic -shared -lhdf5 -lhdf5_hl -lmechanic2 mechanic_module_example.c -o libmechanic_module_example.so



API helpers
-----------

Mechanic comes with a set of useful API functions and macros to read and write data, as
well as attributes and allocate the memory:

#### Memory allocation

#### Reading and writing data

- `MReadData(object, storage_name, buffer)`
- `MWriteData(object, storage_name, buffer)`
  
Both macros take the valid `object`, such as task or pool, the storage bank name `storage_name`,
and read/write data to the `buffer`:

      double ibuffer[1][3];
      double rbuffer[1][3];
      ...
      MWriteData(t, "result", ibuffer);
      ...
      MReadData(t, "result", rbuffer);

  For a dynamically allocated array, we must pass the proper pointer:
      
      double **ibuffer;
      double **rbuffer;
      ...
      MAllocate2(t, "result", ibuffer, double);
      MReadData(t, "result", &ibuffer[0][0]);
      ...
      MAllocate2(t, "result", rbuffer, double);
      MWriteData(t, "result", &rbuffer[0][0]);


**Note: data buffers type must match the datatypes defined through the Storage().**

You may use direct, low-level read/write functions:

- ReadData(storage *s, void *buffer)
- WriteData(storage *s, void *buffer)

These functions take valid storage object, such as `t->storage[0]`, and read/write data
into the buffer:

    double **ibuffer;
    double **rbuffer;
    ...
    MAllocate2(t, "result", ibuffer, double);
    WriteData(t->storage[0], &ibuffer[0][0]); // Assuming "result" is t->storage[0]
    ...
    MAllocate2(t, "result", rbuffer, double);
    ReadData(t->storage[0], &rbuffer[0][0]); // Assuming "result" is t->storage[0]

#### Reading and writing attributes

#### Additional helpers
