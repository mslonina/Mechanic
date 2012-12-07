Mechanic 2.x examples
=====================

List of examples
----------------

1. The `TaskProcess()`:
  - A simple map or image: `mechanic_module_ex_map.c`
  - The Mandelbrot set: `mechanic_module_ex_mandelbrot.c`

2. Working with task pools (using `PoolPrepare()` and `PoolProcess()`):
  - Creating task pools: `mechanic_module_ex_createpool.c`
  - Reading and writing task pool data: `mechanic_module_ex_pool.c`
  - Resetting the task pool: `mechanic_module_ex_reset.c`
  - Different storage layout per task pool (basic): `mechanic_module_ex_chpoollayout.c`
  - Different storage layout per task pool (advanced): `mechanic_module_ex_chpoollayout2.c`

3. Configuration(`Init()`, `Setup()`): 
  - Defining and using configuration options (`Init()` and `Setup()` hooks):
    `mechanic_module_ex_setup.c`

4. Datatypes and dimensionality:
  - Using different datatypes, reading and writing data to the memory banks:
    `mechanic_module_ex_datatypes.c`
  - Using datasets of different datatypes and dimensionality: `mechanic_module_ex_dim.c`

5. Attributes:
  - Using HDF5 attributes: `mechanic_module_ex_attr.c`

6. Advanced hooks:
  - Using `Prepare()` and `Process()` hooks: `mechanic_module_ex_prepareprocess.c`
  - Using `DatasetPrepare()` and `DatasetProcess()`: `mechanic_module_ex_dataset.c`

Compilation
-----------

    mpicc -std=c99 -fPIC -Dpic -shared -lhdf5 -lhdf5_hl -lmechanic2 mechanic_module_example.c -o libmechanic_module_example.so


