Mechanic 2.x quick reference
============================

- [Examples](#examples)
- [The pool loop](#the-pool-loop)
  - [The pool loop explained](#the-pool-loop-explained)
- [The task loop](#the-task-loop)
- [Init](#init)
- [Setup](#setup)
  - [Core options](#core-options)
  - [The configuration file](#the-configuration-file)
- [Storage](#storage)
  - [The pool storage](#the-pool-storage)
  - [The task storage](#the-task-storage)
  - [Accessing the data](#accessing-the-data)
  - [Attributes](#attributes)
  - [Checkpoint](#checkpoint)
- [Datatypes](#datatypes)
- [Hooks](#hooks)
- [API helpers](#api-helpers)
  - [Memory allocation](#memory-allocation)
  - [Reading and writing data](#reading-and-writing-data)
  - [Reading and writing attributes](#reading-and-writing-attributes)
- [Messages](#messages)
- [Error codes](#error-codes)

Examples
--------

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

    mpicc -std=c99 -fPIC -Dpic -shared -lhdf5 -lhdf5_hl -lmechanic2 \
    mechanic_module_example.c -o libmechanic_module_example.so

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


Init
----

The `Init()` hook is used to initialize critical core variables:
- `options` - the number of configuration options (default: 128)
- `pools` - the maximum number of task pools (default: 64)
- `banks_per_pool` - the maximum number of storage banks per task pool (default: 8)
- `banks_per_task` - the maximum number of storage banks per task (default: 8)
- `attr_per_dataset` - the maximum number of attributes per storage bank (default: 24)
- `min_cpu_required` - the minimum number of CPUs to run the job (default: 2)

You may change any of these variables, i.e.

    int Init(init *i) {
      i->min_cpu_required = 4;

      return SUCCESS;
    }

Setup
-----

The configuration is handled by the internal Config API. Options
are defined through the `Setup()` hook. They are available in the command line, as well as
configuration file, and are stored in the master file.

The Config API allows any kind of C99 struct initialization to be used, i.e.:

     int Setup(setup *s) {
       s->options[0] = (options) {
         .space="hello",
         .name="step",
         .shortName="s",
         .value="0.3",
         .type=C_DOUBLE,
         .description="Integration step"
       };
       s->options[1] = (options) {
         .space="hello",
         .name="driver",
         .shortName="\0",
         .value="2",
         .type=C_INT,
         .description="The driver"
       };
       s->options[2] = (options) OPTIONS_END;
       return SUCCESS;
     }


where

- `space` - the name of the configuration namespace (string)
- `name` - the name of the option (string)
- `shortName` - the short name of the option (string), used in the command line,
   may be '\0' (no short option)
- `value` - the default value (string)
- `description` - description for the option (string), used in the command line
- `type` - the option datatype type:
  - `C_INT` - integer option
  - `C_LONG` - long integer option
  - `C_FLOAT` - float option
  - `C_DOUBLE` - double option
  - `C_STRING` - char option
  - `C_VAL` - boolean option

The Config API allows namespaces to contain options with the same name, however, due to
design of the command line (and Popt), the `shortName` must be unique.

The options table must finish with `OPTIONS_END`:

     s->options[2] = (options) OPTIONS_END;

To obtain all available options, try the `--help` or `--usage` flag, i.e.

    mpirun -np 2 mechanic2 -p hello --help
    
Note: Only the master node reads the configuration and parses the commandline.
The configuration is broadcasted then to all nodes.

#### Core options

To obtain all configuration options available in the core, try:

    mpirun -np 2 mechanic2 --help


- `--name`, `-n` - the name of the run
- `--module`, `-p` -- the user-supplied module name
- `--config`, `-c` -- the configuration file name
- `--xres`, `-x` - the task pool board horizontal resolution
- `--yres`, `-y` - the task pool board vertical resolution
- `--zres`, `-z` - the task pool board depth resolution
- `--xmin`, `--xmax` - the task pool board x-axis min/max
- `--ymin`, `--ymax` - the task pool board y-axis min/max
- `--zmin`, `--zmax` - the task pool board z-axis min/max
- `--xorigin` - the task pool board x-axis origin
- `--yorigin` - the task pool board y-axis origin
- `--zorigin` - the task pool board z-axis origin
- `--checkpoint`, `-d` -- the checkpoint size (number of tasks)
- `--checkpoint-files`, `-b` -- the number of incremental backups
- `--no-backup` -- disable automatic master file backup (in case of the same run names)
- `--restart-mode`, `-r` -- the restart mode
- `--restart-file`, -- the restart file
- `--print-defaults` -- print the default options
- `--help`, `-?` -- show help message
- `--usage` -- show short help message

#### The configuration file

All configuration options may be used in a configuration file. The file syntax is:

    [namespace-1]
    option1 = value
    option2 = value

    [namespace-2]
    option3 = value

Everything that starts with `#` is treated as a comment (both single and multiline
comments are supported).

For example, for options defined above, we get the configuration file such as (i.e.
`myconfig.cfg`):

    [hello]
    step = 0.3
    driver = 2


Only one configuration file is needed, for core and module, i.e.

    [hello]
    step = 0.3
    driver = 2

    [core]
    xmin = 0.9
    xmax = 1.1

To use it, we call:

    mpirun -np 2 mechanic2 -p mymodule --config=myconfig.cfg

Of course, we can override default values not only from the configuration file, but also from the
command line:

    mpirun -np 2 mechanic2 -p mymodule --config=myconfig.cfg --driver=1

The configuration order is:

1. The default values defined through the `Setup()`
2. The configuration file
3. The command line

Storage
-------

Mechanic allows to store module data in datasets of any basic datatypes, with minimum rank
2 up to rank `H5S_MAX_RANK` (32). The storage information must be provided with `Storage()` hook.
To define the dataset, any C99 struct initialization is allowed. i.e.:
    
    int Storage(pool *p, setup *s) {
      p->storage[0].layout = (schema) {
        .name = "pool-data",
        .rank = 2,
        .dim[0] = 1, // horizontal size
        .dim[1] = 6, // vertical size
        .use_hdf = 1,
        .storage_type = STORAGE_GROUP,
        .datatype = H5T_NATIVE_DOUBLE,
        .sync = 1,
      };
      return SUCCESS;
    }

where:
 - `name` - the dataset name (string)
 - `rank` - the rank of the dataset (max `H5S_MAX_RANK`)
 - `dim` - the dimensions of the dataset
 - `use_hdf` - whether to store dataset in the master file or not
 - `storage_type` - the type of the storage to use (see below)
 - `sync` - whether to broadcast the data to all computing pool
 - `datatype` - HDF5 datatype 

You can adjust the number of available memory/storage banks by implementing the `Init()`
hook (`banks_per_pool`, `banks_per_task`).

The storage layout may be defined per pool (different storage layout for different task
pools), i.e.

    if (p->pid == 2) {
      p->storage[0].layout.rank = 3;
      p->storage[0].layout.dim[2] = 4;
    }

### The Pool storage

The Pool stores its global data in the `/Pools/pool-ID` group, where the ID is the unique pool identifier.
The task data is stored in `/Pools/pool-ID/Tasks` group.

The size of the storage dataset and the memory is the same. In the case of pool, whole
memory block is stored at once, if `use_hdf = 1` (only `STORAGE_GROUP` is supported for
pools).

The Pool data is broadcasted right after the `PoolPrepare()` hook and stored in the master
datafile.

Note: All global pool datasets must use `STORAGE_GROUP` storage type.

### The task storage

The task data is stored inside `/Pools/pool-ID/Tasks` group. The memory banks defined for
the task storage are synchronized between master and worker after the `TaskProcess()`.

There are four available methods to store the task result:

#### `STORAGE_GROUP`

The whole memory block is stored in a dataset inside `/Tasks/task-ID`
group (the ID is the unique task indentifier), i.e., for a dataset defined similar to:

     p->task->storage[0].layout.name = "basic-dataset";
     p->task->storage[0].layout.dim[0] = 2;
     p->task->storage[0].layout.dim[1] = 6;
     p->task->storage[0].layout.storage_type = STORAGE_GROUP;
     p->task->storage[0].layout.use_hdf = 1;
     p->task->storage[0].layout.datatype = H5T_NATIVE_INT;

the output is stored in `/Pools/pool-ID/Tasks/task-ID/basic-dataset`:

     9 9 9 9 9 9
     9 9 9 9 9 9

#### `STORAGE_PM3D`

The memory block is stored in a dataset with a column-offset, so that
the output is suitable to process with Gnuplot. Example: Suppose we have a 2x5 task
pool:

     1 2 3 4 5
     6 7 8 9 0

while each worker returns the result of size 2x7. For a dataset defined similar to:

     p->task->storage[0].layout.name = "pm3d-dataset";
     p->task->storage[0].layout.dim[0] = 2;
     p->task->storage[0].layout.dim[1] = 7;
     p->task->storage[0].layout.storage_type = STORAGE_PM3D;
     p->task->storage[0].layout.use_hdf = 1;
     p->task->storage[0].layout.datatype = H5T_NATIVE_INT;

we have: `/Pools/pool-ID/Tasks/pm3d-dataset` with:

     1 1 1 1 1 1 1
     1 1 1 1 1 1 1
     6 6 6 6 6 6 6
     6 6 6 6 6 6 6
     2 2 2 2 2 2 2
     2 2 2 2 2 2 2
     7 7 7 7 7 7 7
     7 7 7 7 7 7 7
     ...

The size of the final dataset is `p->pool_size * dim[1]`.

#### `STORAGE_LIST`

The memory block is stored in a dataset with a task-ID offset, This is
similar to `STORAGE_PM3D`, this time however, there is no column-offset. For a dataset
defined as below:

     p->task->storage[0].layout.name = "list-dataset";
     p->task->storage[0].layout.dim[0] = 2;
     p->task->storage[0].layout.dim[1] = 7;
     p->task->storage[0].layout.storage_type = STORAGE_LIST;
     p->task->storage[0].layout.use_hdf = 1;
     p->task->storage[0].layout.datatype = H5T_NATIVE_INT;

the output is stored in `/Pools/pool-ID/Tasks/list-dataset`:

     1 1 1 1 1 1 1
     1 1 1 1 1 1 1
     2 2 2 2 2 2 2
     2 2 2 2 2 2 2
     3 3 3 3 3 3 3
     3 3 3 3 3 3 3
     4 4 4 4 4 4 4
     4 4 4 4 4 4 4
     ...

The size of the final dataset is `p->pool_size * dim[1]`.

#### `STORAGE_BOARD`

The memory block is stored in a dataset with a {row,column,depth}-offset
according to the board-location of the task. The minimum rank must be `TASK_BOARD_RANK`.
Suppose we have a dataset defined like this:

     p->task->storage[0].layout.name = "board-dataset";
     p->task->storage[0].layout.rank = TASK_BOARD_RANK;
     p->task->storage[0].layout.dim[0] = 2;
     p->task->storage[0].layout.dim[1] = 3;
     p->task->storage[0].layout.dim[2] = 1;
     p->task->storage[0].layout.storage_type = STORAGE_BOARD;
     p->task->storage[0].layout.use_hdf = 1;
     p->task->storage[0].layout.datatype = H5T_NATIVE_INT;

For a 2x5 task pool:

     1 2 3 4 5
     6 7 8 9 0

the result is stored in `/Pools/pool-ID/Tasks/board-dataset`:

     1 1 1 2 2 2 3 3 3 4 4 4 5 5 5
     1 1 1 2 2 2 3 3 3 4 4 4 5 5 5
     6 6 6 7 7 7 8 8 8 9 9 9 0 0 0
     6 6 6 7 7 7 8 8 8 9 9 9 0 0 0

The size of the final dataset is `pool_dim[0] * task_dim[0] x pool_dim[1] * task_dim[1]
x pool_dim[2] * task_dim[2] x ... `.
 
### Accessing the data

All data is stored in flattened, one-dimensional arrays, which allows to use different
datatypes and dimensionality of the memory blocks. The data may be accessed directly,
by using the memory pointer:

    p->task->storage[0].memory

You may use `ReadData()` and `WriteData()` or corresponding macros to manipulate the data.
For example, suppose integer-type task dataset of dimensionality dims = {2,3} per task with
storage type `STORAGE_BOARD`, and task board = {5,5}. The allocated memory block is
`p->pool_size x dims x sizeof(int)`. To access it, we need to copy the data:

    int buffer[10][15]; // dims0: 2x5, dims1: 3x5
    ...
    ReadData(&p->task->storage[0], buffer);

(the storage index follows the definition of the datasets).
For a `STORAGE_LIST` or `STORAGE_PM3D`, we would have:
   
    int buffer[50][3]; // dims0: 2x5x5, dims1: 3x5
    ...
    ReadData(&p->task->storage[0], buffer);

For a `STORAGE_GROUP` dataset, the size of the memory block follows the storage
definition:

    int buffer[2][3];
    ...
    // for a pool memory bank
    ReadData(&p->storage[0], buffer);
   
    // or, for a task memory bank, where i is a
    // unique task-ID
    ReadData(&p->tasks[i]->storage[0], buffer);

If you need dynamic allocation of local data buffers, you may use Allocate functions
available. For the 2-dimensional buffers:

    int **buffer;
    ...
    buffer = AllocateInt2(&t->storage[0]);
    ReadData(&t->storage[0], &buff[0][0]);
    ...
    free(buff);


To get the dimensionality of the data block, one can use:

    int dims[MAX_RANK];
    ...
    GetDims(&t->storage[0], dims);

For a reference to allocate and read/write functions take a look at [API Helpers](#api-helpers).

### Attributes

[Introduction to HDF5 attributes](http://www.hdfgroup.org/HDF5/doc/UG/13_Attributes.html)

Attributes are small amount of data that can be attached to datasets. You can attach
attributes to your datasets by defining the proper attribute schema, similar to
dataset's schema:

    p->task->storage[0].attr[0].layout = (schema) {
      .name = "My attribute",
      .dataspace = H5S_SCALAR,
      .datatype = H5T_NATIVE_INT
    }

The code above will attach the integer attribute to the first storage bank of a task.
The `name` is required. In case of more dimensional attributes, you must use `H5S_SIMPLE`
dataspace, and fill out additional information, such as rank and dimensions:

    p->task->storage[0].attr[1].layout = (schema) {
      .name = "My attribute 2D",
      .dataspace = H5S_SIMPLE,
      .datatype = H5T_NATIVE_INT,
      .rank = 2,
      .dim[0] = 3,
      .dim[1] = 4,
    }

By default, 24 attributes are available for each storage bank (both for tasks and
pools). You can change it through the `Init()` hook:

    i->attr_per_dataset = 32;

To write and read attributes, `WriteAttr()` and `ReadAttr()` functions are provided.
Attributes are stored in the master datafile after the `PoolProcess()` hook has been
invoked, so the best place for manipulating them is the `PoolProcess()`:

    int attr_i, attr_d[3][4];
    ...
    WriteAttr(&p->task->storage[0].attr[0], &attr_i);
    WriteAttr(&p->task->storage[0].attr[1], attr_d);

Of course, you can use more advanced techniques with the help of `DatasetPrepare()` and
`DatasetProcess()` hooks, however this requires HDF5 knowledge.

Note: Attributes for `STORAGE_GROUP` are limited, each task group will receive same
attributes (and same values).

Attributes are managed only on the master node.

For a reference to read/write functions take a look at [API Helpers](#api-helpers).

### Checkpoint

The checkpoint contains the results from tasks that have been processed and received. When the
checkpoint is filled up, the result is stored in the master datafile, according to the
storage information provided in the module.

Datatypes
---------

Mechanic support all native MPI/HDF5 datatypes:

| C datatype             | MPI datatype           | HDF5 native datatype |
|:-----------------------|:-----------------------|:---------------------|
| signed char            | MPI_CHAR               | H5T_NATIVE_CHAR      |
| unsigned char          | MPI_UNSIGNED_CHAR      | H5T_NATIVE_UCHAR     |
| signed int             | MPI_INT                | H5T_NATIVE_INT       |
| signed short int       | MPI_SHORT              | H5T_NATIVE_SHORT     |
| signed long int        | MPI_LONG               | H5T_NATIVE_LONG      |
| signed long long int   | MPI_LONG_LONG          | H5T_NATIVE_LLONG     |
| unsigned int           | MPI_UNSIGNED           | H5T_NATIVE_UINT      |
| unsigned short int     | MPI_UNSIGNED_SHORT     | H5T_NATIVE_USHORT    |
| unsigned long long int | MPI_UNSIGNED_LONG_LONG | H5T_NATIVE_ULLONG    |
| float                  | MPI_FLOAT              | H5T_NATIVE_FLOAT     |
| double                 | MPI_DOUBLE             | H5T_NATIVE_DOUBLE    |



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
