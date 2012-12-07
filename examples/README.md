Mechanic 2.x quick start
========================

- [Examples](#list-of-examples)
- [Datatypes](#datatypes)
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


Datatypes
---------

Mechanic support all native MPI/HDF5 datatypes:

| C datatype             | MPI datatype           | HDF5 native datatype | HDF5 platform datatype           |
|:-----------------------|:-----------------------|:---------------------|:---------------------------------|
| signed char            | MPI_CHAR               | H5T_NATIVE_CHAR      | H5T_STD_I8BE or H5T_STD_I8LE     |
| unsigned char          | MPI_UNSIGNED_CHAR      | H5T_NATIVE_UCHAR     | H5T_STD_U8BE or H5T_STD_U8LE     |
| signed int             | MPI_INT                | H5T_NATIVE_INT       | H5T_STD_I32BE or H5T_STD_I32LE   |
| signed short int       | MPI_SHORT              | H5T_NATIVE_SHORT     | H5T_STD_I16BE or H5T_STD_I16LE   |
| signed long int        | MPI_LONG               | H5T_NATIVE_LONG      | H5T_STD_I32BE, H5T_STD_I32LE,    |
|                        |                        |                      | H5T_STD_I64BE or H5T_STD_I64LE   |
| signed long long int   | MPI_LONG_LONG          | H5T_NATIVE_LLONG     | H5T_STD_I64BE or H5T_STD_I64LE   |
| unsigned int           | MPI_UNSIGNED           | H5T_NATIVE_UINT      | H5T_STD_U32BE or H5T_STD_U32LE   |
| unsigned short int     | MPI_UNSIGNED_SHORT     | H5T_NATIVE_USHORT    | H5T_STD_U16BE or H5T_STD_U16LE   |
| unsigned long long int | MPI_UNSIGNED_LONG_LONG | H5T_NATIVE_ULLONG    | H5T_STD_U64BE or H5T_STD_U64LE   |
| float                  | MPI_FLOAT              | H5T_NATIVE_FLOAT     | H5T_IEEE_F32BE or H5T_IEEE_F32LE |
| double                 | MPI_DOUBLE             | H5T_NATIVE_DOUBLE    | H5T_IEEE_F64BE or H5T_IEEE_F64LE |


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

These macros take the valid `object`, the storage bank name `storage_name`, the data
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

The allocation macros allocate memory in one contiguous chunk, so that, only one free is
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


Messages
--------

Error codes
-----------
