Mechanic 2.x quick start
========================

- [How the Mechanic work](#how-the-mechanic-work)
- [How to write a Mechanic module](#how-to-write-a-mechanic-module)
- [Examples](#examples)

How the Mechanic work
---------------------

The goal of the Mechanic is to provide unified framework for scientific applications, that
requires a lot of computing power. Under the hood it uses MPI for CPU communication and
HDF5 standard for data storage. It provides API for common
programming tasks, such as defining and using configuration options, data storage, memory
allocation etc. It was designed this way to simplify and shorten developing process of scientific
codes. The Mechanic does not solve numerical problems, such as NBody or ODEs by self. It
requires the user-supplied module with the actual numerical code. In that way, the core
code is independent on the scientific problem, and the scientific problem is fully
scalable on CPU/GPU clusters. The resulting data is handled in a unified way, through the HDF5 standard,
and may be externally accessed in a number of different applications (say Python).

We tried to design the Mechanic to handle most of scientific problems. It was already
successfully tested with classical NBody problems, RV observation fitting, genetic
algorithms and basic CUDA codes. The connection with user-supplied code takes place in the API hooks,
described shortly on the schematic graph below. None of the hooks is required -- in case of missing hook, 
the corresponding core hook is invoked. No knowledge on MPI/HDF programming is required,
however, it is still possible to use it during advanced hooks (marked with MPI/HDF keyword).

The Mechanic internally works on top of the _MPI Task Farm_ model. This means, we have one
master node which prepares numerical tasks for workers. Tasks are grouped into pools, and
we may have many such pools with different storage and number of tasks. Tasks are mapped
onto 3D task board, which helps to find out which tasks are completed, and is required for
the restart mode. Each pool, as well as tasks, may have own storage to suit best your
application. Different storage types are available, suitable for processing with Gnuplot
or Matplotlib. All basic datatypes are supported and datasets up to rank 32.

The user-supplied module is a C-code compiled to a shared library. It may be
interconnected with any C-interoperable programming language, such as C++, OpenCL, CUDA
and Fortran2003+. Some basic C programming knowledge is required to work with Mechanic
modules.

    -- MPI/HDF init
      
        `- (core) Init()
        `- (core) Setup()
            The core bootstraps and loads sensible defaults. The core looks for the
            user-supplied module specified with -p option

        `- (module) Init()
        `- (module) Setup()
            The user-supplied module bootstraps (if option -p is not specified, the core module is
            loaded as a fallback)

        `- The configuration is read (both core and user-supplied module options)
        `- The master file gets prepared and configuration is stored

        `- [WORK POOL]
            
            `- Prepare() [MPI/HDF]
                Each node invokes the Prepare() hook.

                `- [POOL LOOP] [#1]
                  
                  [CURRENT POOL]

                  The pool consists of tasks. Each pool may have different storage and
                  different number of tasks. Each pool is aware of previous pools and has
                  access to their data. You may use this knowledge as the power at your
                  fingers.

                    `- Storage()
                        All nodes invoke the Storage() hook. The master node prepares
                        specified datasets in the master file. The memory for the current
                        pool is allocated.

                        During dataset creation, the DatasetPrepare() hook is invoked.

                    `- NodePrepare() [MPI/HDF] [#2]
                        All nodes invoke the NodePrepare() hook. The data for the previous
                        pools is available, and might be used to do additional operations,
                        i.e. additional MPI or HDF5 calls.

                    `- PoolPrepare()
                        The master node invokes the PoolPrepare() hook. The data stored in
                        the pool memory is broadcasted to all workers, unless the sync
                        option for the storage bank is set to 0. The data is stored in the
                        master file, unless use_hdf is set to 0.

                        The pool receives p->state = POOL_PREPARED.

                        This is the best place to read additional data files etc. If such
                        is stored in the pool memory, it might be automatically
                        broadcasted to all workers and stored in the master file.

                    `- LoopPrepare() [MPI/HDF]
                        All nodes invoke the LoopPrepare() hook. The pool data is
                        available and might be used to do additional operations, such as
                        MPI/HDF calls.

                        [TASK LOOP]

                          `- TaskBoardMap()
                          `- TaskPrepare()
                              The master node prepares the task. The task receives its
                              location on the task board by the TaskBoardMap().

                          `- TaskProcess()
                              The worker node processes the task. The data stored in the
                              task memory is sended back to the master node. If the
                              checkpoint is filled up, the master node writes the data to
                              the file.

                              [CHECKPOINT]
                                `- CheckpointPrepare()
                                    This hook is invoked on the master node before the
                                    current checkpoint is stored in the master file.

                                    During file storage the DatasetProcess() hook is
                                    invoked. The master file is incrementally backuped at each
                                    checkpoint (i.e., mechanic-master-00.h5,
                                    mechanic-master-01.h5, etc.)
                              [/CHECKPOINT]
                        
                        [/TASK LOOP]

                    `- LoopProcess() [MPI/HDF]
                        The task loop is finished. All received data is available for all
                        nodes. Each node invoke this hook, and it might be used to do
                        additional operations, such as MPI/HDF calls.

                    `- PoolProcess()
                        The master node invoke the PoolProcess() hook. The decision has to
                        be made: POOL_CREATE_NEW will create the new task pool, POOL_RESET
                        will reset the current task pool, the POOL_FINALIZE will finalize
                        the pool loop.

                        In case of POOL_RESET, the loop goes to #2, with reset id
                        p->rid++. The data from the previous revision of the current pool is 
                        available, and may be reused.

                        In case of POOL_CREATE_NEW, the current loop finished, and goes to
                        #1, with the pool id p->pid++. If the p->pid++ is greater than
                        maximum number of pools defined in Init() hook, the pool loop is
                        forced to finalize.

                        In case of POOL_FINALIZE the pool loop is finalized, and goes to
                        #3. The pool receives p->state = POOL_PROCESSED.

                        The pool data as well as task data is stored in the master file.

                    `- NodeProcess() [MPI/HDF]
                        All nodes invoke the NodeProcess() hook, which might be used to do
                        additional operations.

                  [/CURRENT POOL]

                `- [/POOL LOOP] [#3]

            `- Process() [MPI/HDF]
                Each node call the Process() hook.

                The work pool is finalized, and the memory freed.
        `- [/WORK POOL]
        
    -- Finalize



How to write a Mechanic module
------------------------------

The Mechanic does not handle any numerics or science by itself. It requires the user supplied
module. The minimal module must provide storage information and the actual numerical code.

We start by including the neccessary header:

    #include "mechanic.h"

As a simple example, we assume that our numerical task is similar to image processing.
Each worker node will receive coordinates of the image, and process the numerical task
according to those coordinates. Each worker will then send back the 1x3 double-type array, and
the master node will combine all of such arrays into one result. The result will contain
the location of the task and the task id, and will be suitable to process with Gnuplot
PM3D. The storage information for the task is:

    int Storage(pool *p, setup *s) {
      p->task->storage[0].layout = (schema) {
        .name = "result", // the name of the HDF5 dataset
        .rank = 2, // the rank of the dataset
        .dims[0] = 1, // the horizontal size of the task result
        .dims[1] = 3, // the vertical size of the task result
        .datatype = H5T_NATIVE_DOUBLE, // the datatype
        .use_hdf = 1, // whether to store the data in the file or not
        .storage_type = STORAGE_PM3D // the storage type, here, suitable for Gnuplot PM3D
      };
      return SUCCESS;
    }

Finally, we write the `TaskProcess()` function, where the numerics goes on:

    int TaskProcess(pool *p, task *t, setup *s) {
      double buffer[1][3]; // fits the storage information provided in Storage()
      
      buffer[0][0] = t->location[1]; // horizontal position of the task
      buffer[0][1] = t->location[0]; // vertical position of the task
      buffer[0][2] = t->tid; // the task id

      MWriteData(t, "result", &buffer[0][0]);

      return SUCCESS;
    }

We must compile this code to a shared library:

    mpicc -fPiC -Dpic -shared mechanic_module_map.c -o libmechanic_module_map.so

The `libmechanic_module_` prefix is required. We may than run the module:

    mpirun -np 2 mechanic -p map -x 10 -y 20

which will use our code on two nodes (master and one worker), and will do a 10x20
independent numerical simulations, creating a map suitable to process with Gnuplot PM3D.
The data will be stored in the master file: `mechanic-master-00.h5` in the dataset
`/Pools/pool-0000/Tasks/result`.

Examples
--------

As the best start and reference of different Mechanic aspects and possibilites, take a
look at following examples: 

#### The basics

  - A simple map or image:
    [mechanic_module_ex_map.c](./c/mechanic_module_ex_attr.c)
  - The Mandelbrot set:
    [mechanic_module_ex_mandelbrot.c](./c/mechanic_module_ex_mandelbrot.c)

#### Working with task pools

  - Creating task pools:
    [mechanic_module_ex_createpool.c](./c/mechanic_module_ex_createpool.c)
  - Reading and writing task pool data:
    [mechanic_module_ex_pool.c](./c/mechanic_module_ex_pool.c)
  - Resetting the task pool:
    [mechanic_module_ex_reset.c](./c/mechanic_module_ex_reset.c)
  - Different storage layout per task pool (basic):
    [mechanic_module_ex_chpoollayout.c](./c/mechanic_module_ex_chpoollayout.c)
  - Different storage layout per task pool (advanced):
    [mechanic_module_ex_chpoollayout2.c](./c/mechanic_module_ex_chpoollayout2.c)
  - Override the task pool size:
    [mechanic_module_ex_poolsize.c](./c/mechanic_module_ex_poolsize.c)

#### Configuration

  - Defining and using configuration options (`Init()` and `Setup()`):
    [mechanic_module_ex_setup.c](./c/mechanic_module_ex_setup.c)

#### Datatypes and dimensionality

  - Using different datatypes, reading and writing data to the memory banks:
    [mechanic_module_ex_datatypes.c](./c/mechanic_module_ex_datatypes.c)
  - Using datasets of different datatypes and dimensionality:
    [mechanic_module_ex_dim.c](./c/mechanic_module_ex_dim.c)

#### Attributes

  - Using HDF5 attributes:
    [mechanic_module_ex_attr.c](./c/mechanic_module_ex_attr.c)

#### Advanced hooks

  - Using `Prepare()` and `Process()` hooks:
    [mechanic_module_ex_prepareprocess.c](./c/mechanic_module_ex_prepareprocess.c)
  - Using `DatasetPrepare()` and `DatasetProcess()` hooks:
    [mechanic_module_ex_dataset.c](./c/mechanic_module_ex_dataset.c)
  - Using `NodePrepare()` and `NodeProcess()` hooks:
    [mechanic_module_ex_node.c](./c/mechanic_module_ex_node.c)
  - Using `LoopPrepare()` and `LoopProcess()` hooks:
    [mechanic_module_ex_loop.c](./c/mechanic_module_ex_loop.c)

#### Real-life examples

  - Reading input file and storing it in the master datafile (`Storage()` and `PoolPrepare()` hooks):
    [mechanic_module_ex_readfile.c](./c/mechanic_module_ex_readfile.c)
  - Reading input file and storing it in the master datafile with `Setup()`:
    [mechanic_module_ex_readfile_setup.c](./c/mechanic_module_ex_readfile_setup.c)
  - Very simple genetic algorithm:
    [mechanic_module_ex_ga.c](./c/mechanic_module_ex_ga.c)

#### The core module

Take a look at `mechanic_module_core.c` located in `src/modules`. It contains,
document and uses all available hooks.

#### Fortran

  - Connecting external Fortran subroutine:
    [mechanic_module_ex_ffc.c](./fortran/mechanic_module_ex_ffc.c)

#### Compilation

    mpicc -std=c99 -fPIC -Dpic -shared -lhdf5 -lhdf5_hl -lmechanic \
    mechanic_module_example.c -o libmechanic_module_example.so

Mechanic 2.x reference
======================

- [Init](#init)
- [Setup](#setup)
  - [Core options](#core-options)
  - [The configuration file](#the-configuration-file)
  - [Getting the run time configuration](#getting-the-run-time-configuration)
- [Storage](#storage)
  - [The pool storage](#the-pool-storage)
  - [The task storage](#the-task-storage)
  - [Accessing the data](#accessing-the-data)
  - [Attributes](#attributes)
  - [Checkpoint](#checkpoint)
- [Datatypes](#datatypes)
- [The pool loop](#the-pool-loop)
- [Hooks](#hooks)
- [API helpers](#api-helpers)
  - [Memory allocation](#memory-allocation)
  - [Reading and writing data](#reading-and-writing-data)
  - [Reading and writing attributes](#reading-and-writing-attributes)
- [Messages](#messages)
- [Return codes](#return-codes)


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

    mpirun -np 2 mechanic -p hello --help
    
Note: Only the master node reads the configuration and parses the commandline.
The configuration is broadcasted then to all nodes.

#### Core options

To obtain all configuration options available in the core, try:

    mpirun -np 2 mechanic --help


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
- `--xelement` - the task pool board x-axis element
- `--yelement` - the task pool board y-axis element
- `--zelement` - the task pool board z-axis element
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
    step = 0.3 # the step size
    driver = 2 # the integrator


Only one configuration file is needed, for core and module, i.e.

    [hello]
    step = 0.3
    driver = 2

    [core]
    xmin = 0.9
    xmax = 1.1

To use it, we call:

    mpirun -np 2 mechanic -p mymodule --config=myconfig.cfg

Of course, we can override default values not only from the configuration file, but also from the
command line:

    mpirun -np 2 mechanic -p mymodule --config=myconfig.cfg --driver=1

The configuration order is:

1. The default values defined through the `Setup()`
2. The configuration file
3. The command line

#### Getting the run time configuration

Config API provides functions to access the configuration options inside any part of the
module. Options are stored in a linked list, and you may access them through the `setup` object, 
which is passed to all functions, i.e.

    int TaskProcess(pool *p, task *t, setup *s) {
      int driver;
      double xmin, xmax, step;

      driver = Option2Int("hello", "driver", s->head);
      step = Option2Double("hello", "step", s->head);
      xmin = Option2Double("core", "xmin", s->head);
      xmax = Option2Double("core", "xmax", s->head);

      return SUCCESS;
    }

where:

- `int Option2Int(char *space, char *option, configNamespace *head)` (for `C_INT` and `C_VAL`)
- `long Option2Long(char *space, char *option, configNamespace *head)` (for `C_LONG`)
- `float Option2Float(char *space, char *option, configNamespace *head)` (for `C_FLOAT`)
- `double Option2Double(char *space, char *option, configNamespace *head)` (for `C_DOUBLE`)
- `char* Option2String(char *space, char *option, configNamespace *head)` (for `C_STRING`)

Storage
-------

Mechanic allows to store module data in datasets of any basic datatypes, with minimum rank
2 up to rank `H5S_MAX_RANK` (32). The storage information must be provided through the `Storage()` hook.
To define the dataset, any C99 struct initialization is allowed. i.e.:

    int Storage(pool *p, setup *s) {
      p->storage[0].layout = (schema) {
        .name = "sample-data",
        .rank = 2,
        .dims[0] = 1, // horizontal size
        .dims[1] = 6, // vertical size
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
 - `dims` - the dimensions of the dataset
 - `use_hdf` - whether to store dataset in the master file or not
 - `storage_type` - the type of the storage to use (see below)
 - `sync` - whether to broadcast the data to all computing pool
 - `datatype` - HDF5 datatype 

The `storage[0]` is the storage bank index:

    int Storage(pool *p, setup *s) {
      p->storage[0].layout = (schema) {
        .name = "first-dataset",
        .rank = 2,
        .dims[0] = 3,
        .dims[1] = 6,
        .use_hdf = 1,
        .storage_type = STORAGE_GROUP,
        .datatype = H5T_NATIVE_DOUBLE,
        .sync = 1,
      };

      p->storage[1].layout = (schema) {
        .name = "second-dataset",
        .rank = 2,
        .dims[0] = 5,
        .dims[1] = 6,
        .use_hdf = 1,
        .storage_type = STORAGE_GROUP,
        .datatype = H5T_NATIVE_INT,
        .sync = 1,
      };

      return SUCCESS;
    }

By default, there are eight storage banks available, both for the task pools and tasks.
You can adjust the number of available storage banks by implementing the `Init()` hook:

    int Init(init *i) {
      i->banks_per_pool = 12;
      i->banks_per_task = 4;

      return SUCCESS;
    }

The storage layout may be defined per pool (different storage layout for different task
pools), i.e.

    int Storage(pool *p, setup *s) {
      p->storage[0].layout = (schema) {
        .name = "sample-data",
        .rank = 2,
        .dims[0] = 1, // horizontal size
        .dims[1] = 6, // vertical size
        .use_hdf = 1,
        .storage_type = STORAGE_GROUP,
        .datatype = H5T_NATIVE_DOUBLE,
        .sync = 1,
      };

      if (p->pid == 2) {
        p->storage[0].layout.rank = 3;
        p->storage[0].layout.dims[2] = 4;
      }
      return SUCCESS;
    }


### The Pool storage

The Pool stores its global data in the `/Pools/pool-ID` group, where the ID is the unique pool identifier.
The task data is stored in `/Pools/pool-ID/Tasks` group.

The size of the storage dataset and the memory is the same. In the case of pool, whole
memory block is stored at once, if `use_hdf = 1` (only `STORAGE_GROUP` is supported for
pools).

    p->storage[0].layout = (schema) {
      .name = "pool-dataset",
      .rank = 2,
      .dims[0] = 7,
      .dims[1] = 6,
      .use_hdf = 1,
      .storage_type = STORAGE_GROUP,
      .datatype = H5T_NATIVE_INT,
      .sync = 1,
    };

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

    p->task->storage[0].layout = (schema) {
      .name = "basic-dataset",
      .rank = 2,
      .dims[0] = 2,
      .dims[1] = 6,
      .use_hdf = 1,
      .storage_type = STORAGE_GROUP,
      .datatype = H5T_NATIVE_INT,
      .sync = 1,
    };

the output is stored in `/Pools/pool-ID/Tasks/task-ID/basic-dataset`:

     | 9 9 9 9 9 9 |
     | 9 9 9 9 9 9 |

#### `STORAGE_PM3D`

The memory block is stored in a dataset with a column-offset, so that
the output is suitable to process with Gnuplot. Example: Suppose we have a 2x5 task
pool (10 tasks):

     1 2 3 4 5
     6 7 8 9 0

while each worker returns the result of size 2x7. For a dataset defined similar to:

    p->task->storage[0].layout = (schema) {
      .name = "pm3d-dataset",
      .rank = 2,
      .dims[0] = 2,
      .dims[1] = 7,
      .use_hdf = 1,
      .storage_type = STORAGE_PM3D,
      .datatype = H5T_NATIVE_INT,
      .sync = 1,
    };


we have: `/Pools/pool-ID/Tasks/pm3d-dataset` with:

     | 1 1 1 1 1 1 1 | 2x7 datablock
     | 1 1 1 1 1 1 1 | 2x7 x 10 dataset
     |---------------|
     | 6 6 6 6 6 6 6 |
     | 6 6 6 6 6 6 6 |
     |---------------|
     | 2 2 2 2 2 2 2 |
     | 2 2 2 2 2 2 2 |
     |---------------|
     | 7 7 7 7 7 7 7 |
     | 7 7 7 7 7 7 7 |
     | ...

The size of the final dataset is `p->pool_size * dims[1]`.

#### `STORAGE_LIST`

The memory block is stored in a dataset with a task-ID offset, This is
similar to `STORAGE_PM3D`, this time however, there is no column-offset. For a dataset
defined as below:

    p->task->storage[0].layout = (schema) {
      .name = "list-dataset",
      .rank = 2,
      .dims[0] = 2,
      .dims[1] = 7,
      .use_hdf = 1,
      .storage_type = STORAGE_LIST,
      .datatype = H5T_NATIVE_INT,
      .sync = 1,
    };

the output is stored in `/Pools/pool-ID/Tasks/list-dataset`:

     | 1 1 1 1 1 1 1 | 2x7 datablock
     | 1 1 1 1 1 1 1 | 2x7 x 10 dataset
     |---------------|
     | 2 2 2 2 2 2 2 |
     | 2 2 2 2 2 2 2 |
     |---------------|
     | 3 3 3 3 3 3 3 |
     | 3 3 3 3 3 3 3 |
     |---------------|
     | 4 4 4 4 4 4 4 |
     | 4 4 4 4 4 4 4 |
     | ...

The size of the final dataset is `p->pool_size * dims[1]`.

#### `STORAGE_BOARD`

The memory block is stored in a dataset with a {row,column,depth}-offset
according to the board-location of the task. The minimum rank must be `TASK_BOARD_RANK`.
Suppose we have a dataset defined like this:

    p->task->storage[0].layout = (schema) {
      .name = "board-dataset",
      .rank = TASK_BOARD_RANK,
      .dims[0] = 2,
      .dims[1] = 3,
      .dims[2] = 1;
      .use_hdf = 1,
      .storage_type = STORAGE_BOARD,
      .datatype = H5T_NATIVE_INT,
      .sync = 1,
    };

For a 2x5 task pool:

     1 2 3 4 5
     6 7 8 9 0

the result is stored in `/Pools/pool-ID/Tasks/board-dataset`:

     | 1 1 1 | 2 2 2 | 3 3 3 | 4 4 4 | 5 5 5 | 2x3 datablock
     | 1 1 1 | 2 2 2 | 3 3 3 | 4 4 4 | 5 5 5 |
     |-------|-------|-------|-------|-------|
     | 6 6 6 | 7 7 7 | 8 8 8 | 9 9 9 | 0 0 0 |
     | 6 6 6 | 7 7 7 | 8 8 8 | 9 9 9 | 0 0 0 |

The size of the final dataset is `pool_dims[0] * task_dims[0] x pool_dims[1] * task_dims[1]
x pool_dims[2] * task_dims[2] x ... `.
 
### Accessing the data

All data is stored in flattened, one-dimensional arrays (one continguous memory chunk).
This allows to use different datatypes and dimensionality of the memory blocks.
The data may be accessed directly, by using the memory pointer:

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
      .dims[0] = 3,
      .dims[1] = 4,
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
checkpoint is filled up, the result is stored into the master datafile, according to the
storage information provided in the module. You may adjust the number of incremental
checkpoint files with `-b` flag, as well as the size of the checkpoint (number of
completed tasks to store in each checkpoint) with the `-d` flag, i.e.

    mpirun -np 4 mechanic -x 50 -y 50 -b 7 -d 41

The checkpoint size is the main part of the occupied memory resources. It takes into
account all the storage information provided for the tasks. **The checkpoint memory is allocated only
on the master node.** We did our best to optimize
the checkpoint storage, however, there might be cases, where underlying memory resources
are not enough to handle your simulation. In such a case, you should refine the checkpoint
settings (i.e. by reducing the requested number of completed tasks). 


Datatypes
---------

Mechanic supports all native MPI/HDF5 datatypes:

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

The pool loop
-------------

#### The pool states

During the task pool loop, the following status codes are defined, and available through
`p->state`:

- `POOL_PREPARED`, after the `PoolPrepare()` hook has been invoked
- `POOL_PROCESSED`, after the `PoolProcess()` hook has been invoked

i.e.

    NodeProcess(int mpi_size, int node, pool **all, pool *p, setup *s) {
      if (p->state == POOL_PREPARED) {
        ...
      }
      if (p->state == POOL_PROCESSED) {
        ...
      }
      return SUCCESS;
    }

The `PoolProcess()` hook must return one of the following codes:
- `POOL_CREATE_NEW` - the return code for new task pool creation
- `POOL_RESET` - the return code for the current task pool reset
- `POOL_FINALIZE` - the return code to finalize the task pool loop

In the following example, the Pool will be reset 10 times. After that, the next pool will
be created. If the number of pools reach 5, the pool loop is finalized:

    int PoolProcess(pool **all, pool *p, setup *s) {
      if (p->rid < 10) return POOL_RESET;
      if (p->pid < 5) return POOL_CREATE_NEW;
      return POOL_FINALIZE;
    }


Hooks
-----

#### Basic hooks

- `int Init(init *i)` - initialize low level core variables
- `int Setup(setup *s)` - define configuration options
- `int Storage(pool *p, setup *s)` - define the storage layout per task pool
- `int PoolPrepare(pool **all, pool *p, setup *s)` - prepare the current task pool
- `int PoolProcess(pool **all, pool *p, setup *s)` - process the current task pool
- `int TaskPrepare(pool *p, task *t, setup *s)` - prepare the current task
- `int TaskProcess(pool *p, task *t, setup *s)` - process the current task

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
  task loop on the specific node
- `int LoopProcess(int mpi_size, int node, pool **all, pool *p, setup *s)` - process the
  task loop on the specific node

API helpers
-----------

#### Memory allocation

We provide memory allocation macros and functions for 2,3 and 4-dimensional arrays of all
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
      .dims[0] = 4,
      .dims[1] = 5,
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

among with corresponding versions for 3 and 4 dimensions. These functions take the valid
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
      MWriteData(t, "result", &ibuffer[0][0]);
      ...
      MReadData(t, "result", &rbuffer[0][0]);

For a dynamically allocated array:
      
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

The low level interface is available as well:

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

Return codes
------------

Mechanic checks the core and module return codes. In the case of success, the
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

Mechanic will try to safely abort job (with the `MPI_Abort()`) on any error code.

