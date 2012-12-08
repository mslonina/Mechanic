Mechanic 2.x quick reference
============================

- [Examples](#list-of-examples)
- [The pool loop](#the-pool-loop)
- [The task loop](#the-task-loop)
- [Storage](#storage)
- [Datatypes](#datatypes)
- [Hooks](#hooks)
- [API helpers](#api-helpers)
- [Messages](#messages)
- [Error codes](#error-codes)

List of examples
----------------

As a quick start and tutorial for creating numerical modules for the Mechanic, take a look
at following examples:

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

#### Compilation

    mpicc -std=c99 -fPIC -Dpic -shared -lhdf5 -lhdf5_hl -lmechanic2 mechanic_module_example.c -o libmechanic_module_example.so

#### The core module

Take a look at `mechanic_module_core.c` located in `src/modules`. It contains,
documents and uses all available hooks.


The pool loop
-------------

#### The pool loop explained

After the core and the module are bootstrapped, the Mechanic enters the four-step pool
loop:

1. `PoolPrepare()`
  All nodes enter the `PoolPrepare()`. Data of all previous pools is passed to this
  function, so that we may use them to prepare data for the current pool. The current pool
  data is broadcasted to all nodes and stored in the master datafile.

2. The Task Loop
  All nodes enter the task loop. The master node prepares the task during
  `TaskPrepare()`. Each worker receives a task, and calls the `TaskProcess()`. The task
  data, marked with `use_hdf = 1` are received by the master node after the
  `TaskProcess()` is finished and stored during the `CheckpointProcess()`.

3. The Checkpoint
  `The CheckpointPrepare()` hook might be used to adjust the received data. The pool 
  data might be adjusted here as well, the data is stored again during `CheckpointProcess()`.

4. `PoolProcess()`
  After the task loop is finished, the `PoolProcess()` is used to decide whether to
  continue the pool loop or not. The data of all pools is passed to this function. 


#### Task pool states

During the task pool loop, the following status codes are defined, and available through
`p->state`:

- `POOL_PREPARED`, after the `PoolPrepare()` hook has been invoked
- `POOL_PROCESSED`, after the `PoolProcess()` hook has been invoked

i.e.

    NodeProcess(int mpi_size, int node, pool **all, pool *p, setup *s) {
      if (p->status == POOL_PREPARED) {
        ...
      }
      if (p->status == POOL_PROCESSED) {
        ...
      }
    }

The `PoolProcess()` hook must return one of the following codes:
- `POOL_CREATE_NEW` - the return code for new task pool creation
- `POOL_RESET` - the return code for the current task pool reset
- `POOL_FINALIZE` - the return code to finalize the task pool loop


The task loop
-------------

@todo

Storage
-------

@todo

Datatypes
---------

Mechanic support all native MPI/HDF5 datatypes:

| C datatype             | MPI datatype           | HDF5 native datatype | HDF5 platform datatype           |
|:-----------------------|:-----------------------|:---------------------|:---------------------------------|
| signed char            | MPI_CHAR               | H5T_NATIVE_CHAR      | H5T_STD_I8BE or H5T_STD_I8LE     |
| unsigned char          | MPI_UNSIGNED_CHAR      | H5T_NATIVE_UCHAR     | H5T_STD_U8BE or H5T_STD_U8LE     |
| signed int             | MPI_INT                | H5T_NATIVE_INT       | H5T_STD_I32BE or H5T_STD_I32LE   |
| signed short int       | MPI_SHORT              | H5T_NATIVE_SHORT     | H5T_STD_I16BE or H5T_STD_I16LE   |
| signed long int        | MPI_LONG               | H5T_NATIVE_LONG      | H5T_STD_I32BE, H5T_STD_I32LE, H5T_STD_I64BE or H5T_STD_I64LE   |
| signed long long int   | MPI_LONG_LONG          | H5T_NATIVE_LLONG     | H5T_STD_I64BE or H5T_STD_I64LE   |
| unsigned int           | MPI_UNSIGNED           | H5T_NATIVE_UINT      | H5T_STD_U32BE or H5T_STD_U32LE   |
| unsigned short int     | MPI_UNSIGNED_SHORT     | H5T_NATIVE_USHORT    | H5T_STD_U16BE or H5T_STD_U16LE   |
| unsigned long long int | MPI_UNSIGNED_LONG_LONG | H5T_NATIVE_ULLONG    | H5T_STD_U64BE or H5T_STD_U64LE   |
| float                  | MPI_FLOAT              | H5T_NATIVE_FLOAT     | H5T_IEEE_F32BE or H5T_IEEE_F32LE |
| double                 | MPI_DOUBLE             | H5T_NATIVE_DOUBLE    | H5T_IEEE_F64BE or H5T_IEEE_F64LE |



Hooks
-----

#### Basic hooks

- `int Init(init *i)` - initialize low level core variables
- `int Setup(setup *s)` - define configuration options
- `int Storage(pool *p, storage *s)` - define the storage layout per task pool
- `int PoolPrepare(pool **all, pool *p, setup *s)` - prepare the task pool
- `int PoolProcess(pool **all, pool *p, setup *s)` - process the task pool
- `int TaskPrepare(pool *p, task *t, setup *s)` - prepare the task
- `int TaskProcess(pool *p, task *t, setup *s)` - process the task

#### Advanced hooks

- `int TaskBoardMap(pool *p, task *t, setup *s)` - map the task on the task board
- `int CheckpointPrepare(pool *p, checkpoint *c, setup *s)` - prepare the checkpoint 
- `int Prepare(int node, char *masterfile, setup *s)` - prepare the run 
- `int Process(int node, char *masterfile, pool **all, setup *s)` - process the run 
- `int DatasetPrepare(hid_t h5location, hid_t h5dataset, pool *p, storage *d, setup *s)` -
  prepare the dataset
- `int DatasetProcess(hid_t h5location, hid_t h5dataset, pool *p, storage *d, setup *s)` -
  process the dataset
- `int NodePrepare(int mpi_size, int node, pool **all, pool *p, setup *s)` - prepare the
  task pool loop on the specific node
- `int NodeProcess(int mpi_size, int node, pool **all, pool *p, setup *s)` - process the
  task pool loop on the specific node
- `int LoopPrepare(int mpi_size, int node, pool **all, pool *p, setup *s)` - prepare the
  task loop
- `int LoopProcess(int mpi_size, int node, pool **all, pool *p, setup *s)` - process the
  task loop

#### Hooks calling order

    Init()
    Setup()
    Prepare()
      
      [pool loop]
      
        Storage()
          NodePrepare()
          PoolPrepare()
          LoopPrepare()
        
          [task loop]
            TaskBoardMap()
            TaskPrepare()
            TaskProcess()
          [/task loop]
          
          LoopProcess()
          PoolProcess()
          NodeProcess()
        
      [/pool loop]

    Process()

API helpers
-----------

Mechanic comes with a set of useful API functions and macros to read and write data, as
well as attributes and allocate the memory:

#### Memory allocation

We provide memory allocation macros and function for 2,3 and 4-dimensional arrays of all
basic datatypes:

- `MAllocate2(object, storage_name, buffer, datatype)`
- `MAllocate3(object, storage_name, buffer, datatype)`
- `MAllocate4(object, storage_name, buffer, datatype)`

These macros take the valid `object` (pool or task), the storage bank name `storage_name`, the data
buffer `buffer` to allocate and its `datatype`. The datatype must match the datatype
defined through `Storage()`.

For example, have a look at the dataset defined such as:

    p->task->storage[0].layout = (schema) {
      .name = "result",
      .datatype = H5T_NATIVE_DOUBLE,
      .rank = 2,
      .dim[0] = 4,
      .dim[1] = 5,
      ...
    }

During i.e., `TaskProcess()` we want the result buffer to match the defined storage:

    double **buffer;
    ...
    MAllocate2(t, "result", buffer, double);

This will guarantee that the allocated buffer has enough space to hold data for the
"result" storage bank.

The allocation macros **allocate memory in one contiguous chunk**, so that, only one free is
required:

    free(buffer);

Low-level memory allocation functions are available:

- `AllocateInt2(storage *s)` for `int`
- `AllocateShort2(storage *s)` for `short`
- `AllocateLong2(storage *s)` for `long`
- `AllocateLLong2(storage *s)` for `long long`
- `AllocateUInt2(storage *s)` for `unsigned int`
- `AllocateUShort2(storage *s)` for `unsigned short`
- `AllocateULLong2(storage *s)` for `unsigned long long`
- `AllocateFloat2(storage *s)` for `float`
- `AllocateDouble2(storage *s)` for `double`

among with corresponding versions for 3 and 4 dimensions. These functions takes the valid
storage object, i.e.

    int **ibuffer;
    ibuffer = AllocateInt2(t->storage[0]);
    ...
    free(ibuffer);

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

- `int ReadData(storage *s, void *buffer)`
- `int WriteData(storage *s, void *buffer)`

These functions take valid storage object, such as `t->storage[0]`, and read/write data
into the buffer:

    double **ibuffer;
    double **rbuffer;
    ...
    MAllocate2(t, "result", ibuffer, double);
    WriteData(&t->storage[0], &ibuffer[0][0]); // Assuming "result" is t->storage[0]
    ...
    MAllocate2(t, "result", rbuffer, double);
    ReadData(&t->storage[0], &rbuffer[0][0]); // Assuming "result" is t->storage[0]

#### Reading and writing attributes

- `MReadAttr(object, storage_name, attribute_name, buffer)`
- `MWriteAttr(object, storage_name, attribute_name, buffer)`
  
Both macros take the valid `object`, such as task or pool, the storage bank name `storage_name`,
the attribute name `attribute_name` and read/write the attribute data to the `buffer`:
  
    int iattr, rattr;
    iattr = 91;
    MWriteAttr(t, "dataset", "integer attr", &iattr);
    ...
    MReadAttr(t, "dataset", "integer attr", &rattr);

The low level interface is available:

- `int WriteAttr(storage *s, void *buffer)`
- `int ReadAttr(storage *s, void *buffer)`

i.e.

    int iattr, rattr;
    iattr = 91;
    WriteAttr(&t->storage[0].attr[0], &iattr);
    ...
    ReadAttr(&t->storage[0].attr[0], &rattr);


#### Additional helpers

- `MGetDims(object, storage_name, dims)`
  
This macro gets the dimensions of the specified 
storage bank `storage_name` of a given `object` (pool or task), i.e.

    int dims[MAX_RANK];
    MGetDims(t, "result", dims);

Low level interface is available:
- `int GetDims(storage *s, int *dims)`

i.e.

    int dims[MAX_RANK];
    GetDims(t->storage[0], dims);

**Note: the dims array must be at least of length `MAX_RANK`.**

Messages
--------

We provide wrapper for `printf` for prettier handling of messages:

- `Message(message_type, format, args)`

i.e.

    Message(MESSAGE_INFO, "my pretty message from node %d\n", node);

Available types of messages:

- `MESSAGE_INFO`
- `MESSAGE_ERR`
- `MESSAGE_WARN`
- `MESSAGE_OUTPUT`
- `MESSAGE_RESULT`
- `MESSAGE_COMMENT`

Error codes
-----------

Mechanic check the internal as well as module return codes. In the case of success, the
function should return `SUCCESS` code, i.e.

    TaskProcess(pool *p, task *t, setup *s) {
      return SUCCESS;
    }

Otherwise, following error codes should be returned:

| Module error code | Code | Core error code | Code | Component   |
|:------------------|:-----|:----------------|:-----|:------------|
| MODULE_ERR_CORE   | 801  | CORE_ERR_CORE   | 901  | Core        |
| MODULE_ERR_MPI    | 811  | CORE_ERR_MPI    | 911  | MPI         |  
| MODULE_ERR_HDF    | 812  | CORE_ERR_HDF    | 912  | HDF5        |
| -                 | -    | CORE_ERR_MODULE | 913  | Module      |
| MODULE_ERR_SETUP  | 814  | CORE_ERR_SETUP  | 914  | Setup       |
| MODULE_ERR_MEM    | 815  | CORE_ERR_MEM    | 915  | Memory      |
| MODULE_ERR_STORAGE| 816  | CORE_ERR_STORAGE| 916  | Storage     |
| MODULE_ERR_OTHER  | 888  | CORE_ERR_OTHER  | 999  | Other       |

Mechanic will try to safely abort job on any error code.
