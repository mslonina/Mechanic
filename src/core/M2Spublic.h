/**
 * @file
 * Data storage and management (public API)
 */
#ifndef MECHANIC_M2S_PUBLIC_H
#define MECHANIC_M2S_PUBLIC_H

#include <string.h>
#include <hdf5.h>
#include <mpi.h>

#include "M2Epublic.h"
#include "M2Mpublic.h"

/* Storage */
#define MAX_RANK H5S_MAX_RANK /**< The maximum dataset rank */
#define STORAGE_NULL 0 /**< The NULL storage */
#define STORAGE_GROUP 11 /**< The basic data storage type */
#define STORAGE_PM3D 12 /**< The pm3d data storage type */
#define STORAGE_TEXTURE 13 /**< The board data storage type */
#define STORAGE_LIST 14 /**< The list data storage type */

#define HDF_NO_STORAGE 0 /**< No HDF5 file storage */
#define HDF_NORMAL_STORAGE 1 /**< HDF5 file storage */
#define HDF_TEMP_STORAGE 2 /**< Only temporary HDF5 file storage */

#define STORAGE_END {.name = NULL, .dataspace = H5S_SIMPLE, .datatype = -1, .mpi_datatype = MPI_DOUBLE, .rank = 0, .dims = {0, 0, 0, 0}, .offsets = {0, 0, 0, 0}, .use_hdf = 0, .sync = 0, .storage_type = STORAGE_NULL} /**< The storage scheme default initializer */
#define FIELD_STORAGE_END {.name = NULL, .dataspace = H5S_SIMPLE, .datatype = -1, .mpi_datatype = MPI_DOUBLE, .rank = 0, .dims = {0, 0, 0, 0}, .offsets = {0, 0, 0, 0}, .use_hdf = 0, .sync = 0, .storage_type = STORAGE_NULL, .field_offset = -1} /**< The storage scheme default initializer */
#define ATTR_STORAGE_END {.name = NULL, .dataspace = H5S_NO_CLASS, .datatype = -1, .mpi_datatype = MPI_DOUBLE, .rank = 0, .dims = {0, 0, 0, 0}, .offsets = {0, 0, 0, 0}, .use_hdf = 0, .sync = 0, .storage_type = STORAGE_NULL} /**< The attribute storage scheme default initializer */

/**
 * @struct schema
 * Defines the memory/storage schema
 */
typedef struct {
  char *name; /**< The name of the dataset */
  unsigned short rank; /**< The rank of the dataset */
  unsigned short storage_type; /**< The storage type: STORAGE_GROUP, STORAGE_PM3D, STORAGE_TEXTURE, STORAGE_LIST */
  unsigned short use_hdf; /**< Enables HDF5 storage for the memory block */
  unsigned short sync; /**< Whether to synchronize memory bank between master and worker */
  unsigned int dims[MAX_RANK]; /**< The dimensions of the memory dataset */
  hid_t datatype; /**< The datatype of the dataset */
  unsigned int storage_dim[MAX_RANK]; /**< @internal The dimensions of the storage dataset */
  unsigned int offsets[MAX_RANK]; /**< @internal The offsets (calculated automatically) */
  H5S_class_t dataspace; /**< @internal The type of the HDF5 dataspace (H5S_SIMPLE) */
  MPI_Datatype mpi_datatype; /**< @internal The MPI datatype of the dataset */
  size_t size; /**< @internal The size of the memory block */
  size_t storage_size; /**< @internal The size of the storage block */
  size_t datatype_size; /** @internal The size of the datatype */
  unsigned int elements; /**< @internal Number of data elements in the memory block */
  unsigned int storage_elements; /**< @internal Number of data elements in the storage block */
  size_t compound_size; /**< @internal Compound datatype size */
  size_t field_offset; /**< Compound datatype field offset */
} schema;

/**
 * @struct field
 * Defines the compound datatype field schema
 */
typedef struct {
  schema layout;
} field;

/**
 * @struct attr
 * Defines the attribute memory/storage schema
 */
typedef struct {
  schema layout; /**< The memory/storage schema, @see schema */
  unsigned char *memory; /**< The memory block */
  field *field; /**< Compound datatype fields */
  unsigned short compound_fields; /**< Number of compound datatype fields in use */
} attr;

/**
 * @struct storage
 * The storage structure
 */
typedef struct {
  schema layout; /**< The memory/storage schema, @see schema */
  unsigned char *memory; /**< The memory block */
  attr *attr; /**< The dataset attributes */
  field *field; /**< Compound datatype fields */
  unsigned short attr_banks; /**< Number of attribute banks in use */
  unsigned short compound_fields; /**< Number of compound datatype fields in use */
} storage;

/**
 * @struct task
 * The task
 */
typedef struct {
  unsigned int pid; /**< The parent pool id */
  unsigned int tid; /**< The task id */
  unsigned int rid; /**< The task reset id */
  unsigned int cid; /**< The task checkpoint id */
  short status; /**< The task status */
  short state; /**< The task processing state */
  unsigned int location[TASK_BOARD_RANK]; /**< Coordinates of the task */
  unsigned short node; /** The computing node */
  storage *storage; /**< The storage schema and data */
} task;

/**
 * @struct pool
 * The pool
 */
typedef struct {
  unsigned int pid; /**< The pool id */
  unsigned int rid; /**< The pool reset id */
  unsigned int sid; /**< The stage id */
  unsigned int srid; /**< The stage reset id */
  short status; /**< The pool create status */
  short state; /**< The pool processing state */
  storage *board; /**< The task board */
  storage *storage; /**< The global pool storage scheme */
  task *task; /**< The task scheme */
  task **tasks; /**< All tasks */
  unsigned int checkpoint_size; /**< The checkpoint size */
  unsigned int pool_size; /**< The pool size (number of tasks to do) */
  unsigned int mask_size; /**< The mask size (number of tasks to mask on a given reset loop) */
  unsigned int completed; /**< The pool task completed counter */
  unsigned short node; /**< The node ID */
  int mpi_size; /**< The MPI COMM size */
  unsigned short pool_banks; /**< The number of pool memory banks */
  unsigned short task_banks; /**< The number of task memory banks */
  unsigned short attr_banks; /**< The number of attributes banks */
} pool;

/**
 * Data read/write helpers
 */
unsigned int GetSize(unsigned int rank, unsigned int *dims); /**< Get the 1D size for given rank and dimensions */
void GetDims(storage *s, unsigned int *dims); /**< Get the dimensions of the storage object */
int CopyData(void *in, void *out, size_t size); /**< Copy data buffers */

/**
 * Direct read/write interface
 */
int WriteData(storage *s, void *data); /**< Copy local data buffers to memory (by storage index) */
int ReadData(storage *s, void *data); /**< Copy memory buffers to local data buffers (by storage index) */
int WriteAttr(attr *a, void *data); /**< Copy local attribute buffers to memory (by attribute index) */
int ReadAttr(attr *a, void *data); /**< Copy attribute buffers to local data buffers (by attribute index */

int GetStorageIndex(storage *s, char *storage_name); /**< Get the index for given storage bank */
int GetAttributeIndex(attr *a, char *storage_name); /**< Get the index for given attribute */

/**
 * @macro
 * Read the data for the given object (pool, task)
 */
#define MReadData(_mobject, _mstorage_name, _mdata)\
  if (_mobject) {\
    int _msindex, _mmstat;\
    _msindex = GetStorageIndex(_mobject->storage, _mstorage_name);\
    if (_msindex < 0) {\
      Message(MESSAGE_ERR, "MReadData: Storage bank '%s' could not be found\n", _mstorage_name);\
      Error(CORE_ERR_MEM);\
    } else {\
      _mmstat = ReadData(&_mobject->storage[_msindex], _mdata);\
      CheckStatus(_mmstat);\
    }\
  } else {\
    Message(MESSAGE_ERR, "MReadData: Invalid object\n");\
    Error(CORE_ERR_MEM);\
  }

/**
 * @macro
 * Write the data for the given object (pool, task)
 */
#define MWriteData(_mobject, _mstorage_name, _mdata)\
  if (_mobject) {\
    int _msindex, _mmstat;\
    _msindex = GetStorageIndex(_mobject->storage, _mstorage_name);\
    if (_msindex < 0) {\
      Message(MESSAGE_ERR, "MWriteData: Storage bank '%s' could not be found\n", _mstorage_name);\
      Error(CORE_ERR_MEM);\
    } else {\
      _mmstat = WriteData(&_mobject->storage[_msindex], _mdata);\
      CheckStatus(_mmstat);\
    }\
  } else {\
    Message(MESSAGE_ERR, "MWriteData: Invalid object\n");\
    Error(CORE_ERR_MEM);\
  }

/**
 * @macro
 * Read the attribute for the given object (pool, task)
 */
#define MReadAttr(_mobject, _mstorage_name, _mattr_name, _mdata)\
  if (_mobject) {\
    int _msindex, _maindex, _mmstat;\
    _msindex = GetStorageIndex(_mobject->storage, _mstorage_name);\
    if (_msindex < 0) {\
      Message(MESSAGE_ERR, "MReadAttr: Storage bank '%s' could not be found\n", _mstorage_name);\
      Error(CORE_ERR_MEM);\
    } else {\
      _maindex = GetAttributeIndex(_mobject->storage[_msindex].attr, _mattr_name);\
      if (_maindex < 0) {\
        Message(MESSAGE_ERR, "MReadAttr: Attribute '%s' for storage '%s' could not be found\n",\
            _mattr_name, _mstorage_name);\
        Error(CORE_ERR_MEM);\
      }\
      _mmstat = ReadAttr(&_mobject->storage[_msindex].attr[_maindex], _mdata);\
      CheckStatus(_mmstat);\
    }\
  } else {\
    Message(MESSAGE_ERR, "MReadAttr: Invalid object\n");\
    Error(CORE_ERR_MEM);\
  }

/**
 * @macro
 * Write the attribute for the given object (pool, task)
 */
#define MWriteAttr(_mobject, _mstorage_name, _mattr_name, _mdata)\
  if (_mobject) {\
    int _msindex, _maindex, _mmstat;\
    _msindex = GetStorageIndex(_mobject->storage, _mstorage_name);\
    if (_msindex < 0) {\
      Message(MESSAGE_ERR, "MWriteAttr: Storage bank '%s' could not be found\n", _mstorage_name);\
      Error(CORE_ERR_MEM);\
    } else {\
      _maindex = GetAttributeIndex(_mobject->storage[_msindex].attr, _mattr_name);\
      if (_maindex < 0) {\
        Message(MESSAGE_ERR, "MWriteAttr: Attribute '%s' for storage '%s' could not be found\n",\
            _mattr_name, _mstorage_name);\
        Error(CORE_ERR_MEM);\
      }\
      _mmstat = WriteAttr(&_mobject->storage[_msindex].attr[_maindex], _mdata);\
      CheckStatus(_mmstat);\
    }\
  } else {\
    Message(MESSAGE_ERR, "MWriteAttr: Invalid object\n");\
    Error(CORE_ERR_MEM);\
  }

/**
 * @macro
 * Get the dimensions for the given object
 */
#define MGetDims(_mobject, _mstorage_name, _dims)\
  if (_mobject) {\
    int _msindex;\
    _msindex = GetStorageIndex(_mobject->storage, _mstorage_name);\
    if (_msindex < 0) {\
      Message(MESSAGE_ERR, "MGetDims: Storage bank '%s' could not be found\n", _mstorage_name);\
      Error(CORE_ERR_MEM);\
    } else {\
      GetDims(&_mobject->storage[_msindex], _dims);\
    }\
  } else {\
    Message(MESSAGE_ERR, "MGetDims: Invalid object\n");\
    Error(CORE_ERR_MEM);\
  }

/**
 * Low level Memory allocation helpers
 */

/* 2D */
int** AllocateInt2(storage *s);
short** AllocateShort2(storage *s);
long** AllocateLong2(storage *s);
long long** AllocateLLong2(storage *s);
unsigned int** AllocateUInt2(storage *s);
unsigned short** AllocateUShort2(storage *s);
unsigned long long** AllocateULLong2(storage *s);
float** AllocateFloat2(storage *s);
double** AllocateDouble2(storage *s);

/* 3D */
int*** AllocateInt3(storage *s);
short*** AllocateShort3(storage *s);
long*** AllocateLong3(storage *s);
long long*** AllocateLLong3(storage *s);
unsigned int*** AllocateUInt3(storage *s);
unsigned short*** AllocateUShort3(storage *s);
unsigned long long*** AllocateULLong3(storage *s);
float*** AllocateFloat3(storage *s);
double*** AllocateDouble3(storage *s);

/* 4D */
int**** AllocateInt4(storage *s);
short**** AllocateShort4(storage *s);
long**** AllocateLong4(storage *s);
long long**** AllocateLLong4(storage *s);
unsigned int**** AllocateUInt4(storage *s);
unsigned short**** AllocateUShort4(storage *s);
unsigned long long**** AllocateULLong4(storage *s);
float**** AllocateFloat4(storage *s);
double**** AllocateDouble4(storage *s);

/* 5D */
int***** AllocateInt5(storage *s);
short***** AllocateShort5(storage *s);
long***** AllocateLong5(storage *s);
long long***** AllocateLLong5(storage *s);
unsigned int***** AllocateUInt5(storage *s);
unsigned short***** AllocateUShort5(storage *s);
unsigned long long***** AllocateULLong5(storage *s);
float***** AllocateFloat5(storage *s);
double***** AllocateDouble5(storage *s);


int Allocate(storage *s, size_t size, size_t datatype); /**< Memory allocator */
int AllocateAttribute(attr *s, size_t size, size_t datatype); /**< Memory allocator */

/**
 * High level memory allocation macros
 */

/**
 * @macro
 * Allocate the 2D contiguous array
 */
#define MAllocate2(_mobject, _mstorage_name, _mbuffer, _mtype)\
  if (_mobject) {\
    int _msindex;\
    unsigned int _i = 0, _dim0, _dim1;\
    _msindex = GetStorageIndex(_mobject->storage, _mstorage_name);\
    if (_msindex < 0) {\
      Message(MESSAGE_ERR, "MAllocate2: Storage bank '%s' could not be found\n", _mstorage_name);\
      Error(CORE_ERR_MEM);\
    } else {\
      _dim0 = _mobject->storage[_msindex].layout.storage_dim[0];\
      _dim1 = _mobject->storage[_msindex].layout.storage_dim[1];\
      if (_mobject->storage[_msindex].layout.storage_size > 0) {\
        _mbuffer = malloc((_dim0 * sizeof(_mtype*)) + (_dim0 * _dim1 * sizeof(_mtype)));\
        if (_mbuffer) {\
          for (_i = 0; _i < _dim0; _i++) {\
            _mbuffer[_i] = (_mtype*)(_mbuffer + _dim0) + _i * _dim1;\
          }\
        } else {\
          Error(CORE_ERR_MEM);\
        }\
      }\
    }\
  } else {\
    Message(MESSAGE_ERR, "MAllocate2: Invalid object\n");\
    Error(CORE_ERR_MEM);\
  }

/**
 * @macro
 * Allocate the 3D contiguous array
 */
#define MAllocate3(_mobject, _mstorage_name, _mbuffer, _mtype)\
  if (_mobject) {\
    int _msindex;\
    unsigned int _i = 0, _j = 0, _dim0, _dim1, _dim2;\
    _msindex = GetStorageIndex(_mobject->storage, _mstorage_name);\
    if (_msindex < 0) {\
      Message(MESSAGE_ERR, "MAllocate3: Storage bank '%s' could not be found\n", _mstorage_name);\
      Error(CORE_ERR_MEM);\
    } else {\
      _dim0 = _mobject->storage[_msindex].layout.storage_dim[0];\
      _dim1 = _mobject->storage[_msindex].layout.storage_dim[1];\
      _dim2 = _mobject->storage[_msindex].layout.storage_dim[2];\
      if (_mobject->storage[_msindex].layout.storage_size > 0) {\
        _mbuffer = malloc((_dim0 * sizeof(_mtype*)) + (_dim0 * _dim1 * sizeof(_mtype**)) + (_dim0 * _dim1 * _dim2 * sizeof(_mtype)));\
        if (_mbuffer) {\
          for (_i = 0; _i < _dim0; _i++) {\
            _mbuffer[_i] = (_mtype**)(_mbuffer + _dim0) + _i * _dim1;\
            for (_j = 0; _j < _dim1; _j++) {\
              _mbuffer[_i][_j] = (_mtype*)(_mbuffer + _dim0 + _dim0 * _dim1) + _i * _dim1 * _dim2 + _j * _dim2;\
            }\
          }\
        } else {\
          Error(CORE_ERR_MEM);\
        }\
      }\
    }\
  } else {\
    Message(MESSAGE_ERR, "MAllocate3: Invalid object\n");\
    Error(CORE_ERR_MEM);\
  }

/**
 * @macro
 * Allocate the 4D contiguous array
 */
#define MAllocate4(_mobject, _mstorage_name, _mbuffer, _mtype)\
  if (_mobject) {\
    int _msindex;\
    unsigned int _i = 0, _j = 0, _k = 0, _dim0, _dim1, _dim2, _dim3;\
    _msindex = GetStorageIndex(_mobject->storage, _mstorage_name);\
    if (_msindex < 0) {\
      Message(MESSAGE_ERR, "MAllocate4: Storage bank '%s' could not be found\n", _mstorage_name);\
      Error(CORE_ERR_MEM);\
    } else {\
      _dim0 = _mobject->storage[_msindex].layout.storage_dim[0];\
      _dim1 = _mobject->storage[_msindex].layout.storage_dim[1];\
      _dim2 = _mobject->storage[_msindex].layout.storage_dim[2];\
      _dim3 = _mobject->storage[_msindex].layout.storage_dim[3];\
      if (_mobject->storage[_msindex].layout.storage_size > 0) {\
        _mbuffer = malloc((_dim0 * sizeof(_mtype*)) + (_dim0 * _dim1 * sizeof(_mtype**)) + (_dim0 * _dim1 * _dim2 * sizeof(_mtype***))\
          + (_dim0 * _dim1 * _dim2 * _dim3 * sizeof(_mtype)));\
        if (_mbuffer) {\
          for (_i = 0; _i < _dim0; _i++) {\
            _mbuffer[_i] = (_mtype***)(_mbuffer + _dim0) + _i * _dim1;\
            for (_j = 0; _j < _dim1; _j++) {\
              _mbuffer[_i][_j] = (_mtype**)(_mbuffer + _dim0 + _dim0 * _dim1) + _i * _dim1 * _dim2 + _j * _dim2;\
              for (_k = 0; _k < _dim2; _k++) {\
                _mbuffer[_i][_j][_k] = (_mtype*)(_mbuffer + _dim0 + _dim0 * _dim1 + _dim0 * _dim1 * _dim2) + \
                  _i * _dim1 * _dim2 * _dim3 + _j * _dim2 * _dim3 + _k * _dim3;\
              }\
            }\
          }\
        } else {\
          Error(CORE_ERR_MEM);\
        }\
      }\
    }\
  } else {\
    Message(MESSAGE_ERR, "MAllocate4: Invalid object\n");\
    Error(CORE_ERR_MEM);\
  }

/**
 * @macro
 * Allocate the 5D contiguous array
 */
#define MAllocate5(_mobject, _mstorage_name, _mbuffer, _mtype)\
  if (_mobject) {\
    int _msindex;\
    unsigned int _i = 0, _j = 0, _k = 0, _l = 0, _dim0, _dim1, _dim2, _dim3, _dim4;\
    _msindex = GetStorageIndex(_mobject->storage, _mstorage_name);\
    if (_msindex < 0) {\
      Message(MESSAGE_ERR, "MAllocate5: Storage bank '%s' could not be found\n", _mstorage_name);\
      Error(CORE_ERR_MEM);\
    } else {\
      _dim0 = _mobject->storage[_msindex].layout.storage_dim[0];\
      _dim1 = _mobject->storage[_msindex].layout.storage_dim[1];\
      _dim2 = _mobject->storage[_msindex].layout.storage_dim[2];\
      _dim3 = _mobject->storage[_msindex].layout.storage_dim[3];\
      _dim4 = _mobject->storage[_msindex].layout.storage_dim[4];\
      if (_mobject->storage[_msindex].layout.storage_size > 0) {\
        _mbuffer = malloc((_dim0 * sizeof(_mtype*)) + (_dim0 * _dim1 * sizeof(_mtype**)) + (_dim0 * _dim1 * _dim2 * sizeof(_mtype***))\
            + (_dim0 * _dim1 * _dim2 * _dim3 * sizeof(_mtype****)) + (_dim0 * _dim1 * _dim2 * _dim3 * _dim4 * sizeof(_mtype)));\
        if (_mbuffer) {\
          for (_i = 0; _i < _dim0; _i++) {\
            _mbuffer[_i] = (_mtype****)(_mbuffer + _dim0) + _i * _dim1;\
            for (_j = 0; _j < _dim1; _j++) {\
              _mbuffer[_i][_j] = (_mtype***)(_mbuffer + _dim0 + _dim0 * _dim1) + _i * _dim1 * _dim2 + _j * _dim2;\
              for (_k = 0; _k < _dim2; _k++) {\
                _mbuffer[_i][_j][_k] = (_mtype**)(_mbuffer + _dim0 + _dim0 * _dim1 + _dim0 * _dim1 * _dim2) + \
                  _i * _dim1 * _dim2 * _dim3 + _j * _dim2 * _dim3 + _k * _dim3;\
                for (_l = 0; _l < _dim3; _l++) {\
                  _mbuffer[_i][_j][_k][_l] = (_mtype*)(_mbuffer + _dim0 + _dim0 * _dim1 + _dim0 * _dim1 * _dim2 + _dim0 * _dim1 * _dim2 * _dim3) + \
                    _i * _dim1 * _dim2 * _dim3 * _dim4 + _j * _dim2 * _dim3 * _dim4 + _k * _dim3 * _dim4 + _l * _dim4;\
                }\
              }\
            }\
          }\
        } else {\
          Error(CORE_ERR_MEM);\
        }\
      }\
    }\
  } else {\
    Message(MESSAGE_ERR, "MAllocate5: Invalid object\n");\
    Error(CORE_ERR_MEM);\
  }

int CommitMemoryLayout(int banks, storage *s);
int CommitAttrMemoryLayout(int banks, storage *s);
int CommitAttribute(hid_t h5location, attr *a);
void FreeMemoryLayout(unsigned int banks, unsigned int attr_banks, storage *s);

int CommitData(hid_t h5location, int banks, storage *s);
int ReadDataset(hid_t h5location, int banks, storage *s, unsigned int size);

size_t GetPadding(unsigned int elements, size_t datatype_size);
hid_t CommitDatatype(storage *s);
hid_t CommitFileDatatype(storage *s);
hid_t CommitAttrFileDatatype(attr *s);

#endif
