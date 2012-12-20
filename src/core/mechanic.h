/**
 * @file
 * The Mechanic2 public API
 */
#ifndef MECHANIC_MECHANIC_H
#define MECHANIC_MECHANIC_H

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/dir.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <fcntl.h>
#include <time.h>
#include <ctype.h>
#include <dlfcn.h>
#include <math.h>
#include <popt.h>
#include <mpi.h>
#include <hdf5.h>
#include <hdf5_hl.h>

#define ICE_FILENAME "mechanic.ice" /** The Mechanic ICE file */

#define SUCCESS 0 /**< The success return code */
#define CORE_ICE 112 /**< The core emergency return code */
#define CORE_SETUP_HELP 212 /**< The core help message return code */
#define CORE_SETUP_USAGE 213 /**< The core usage message return code */

/* Error codes */
#define CORE_ERR_CORE 901 /**< The core-related error */
#define CORE_ERR_MPI 911 /**< The core MPI-related error */
#define CORE_ERR_HDF 912 /**< The core HDF-related error */
#define CORE_ERR_MODULE 913 /**< The core module-related error */
#define CORE_ERR_SETUP 914 /**< The core setup-related error */
#define CORE_ERR_MEM 915 /**< The core memory-related error */
#define CORE_ERR_CHECKPOINT 916 /**< The core checkpoint-related error */
#define CORE_ERR_STORAGE 917 /**< The core storage-related error */
#define CORE_ERR_OTHER 999 /**< The core any other error */

#define MODULE_ERR_CORE 801 /**< The module-related error */
#define MODULE_ERR_MPI 811 /**< The module MPI-related error */
#define MODULE_ERR_HDF 812 /**< The module HDF-related error */
#define MODULE_ERR_SETUP 814 /**< The module setup-related error */
#define MODULE_ERR_MEM 815 /**< The module memory-related error */
#define MODULE_ERR_CHECKPOINT 816 /**< The module checkpoint-related error */
#define MODULE_ERR_OTHER 888 /**< The module any other error */

/* Pool */
#define POOL_FINALIZE 1001 /**< The pool finalize return code */
#define POOL_RESET 1002 /**< The pool reset return code */
#define POOL_CREATE_NEW 1003 /**< The pool create new return code */
#define POOL_PREPARED 2001 /**< Pool prepared state */
#define POOL_PROCESSED 2002 /**< Pool processed state */

/* Storage */
#define MAX_RANK H5S_MAX_RANK /**< The maximum dataset rank */
#define TASK_BOARD_RANK 3 /**< The maximum task board rank */
#define STORAGE_GROUP 11 /**< The basic data storage type */
#define STORAGE_PM3D 12 /**< The pm3d data storage type */
#define STORAGE_BOARD 13 /**< The board data storage type */
#define STORAGE_LIST 14 /**< The list data storage type */

/* Task */
#define TASK_EMPTY -88 /**< The task empty return code */
#define TASK_FINISHED 1 /**< The task finished return code */
#define TASK_AVAILABLE 0 /**< The task available return code */
#define TASK_IN_USE -1 /**< The task in use return code */
#define TASK_TO_BE_RESTARTED -2 /**< The task to be restarted return code */
#define NO_MORE_TASKS -99 /**< No more tasks return code */

/* Message tags */
#define TAG_DATA 1337 /**< Send/Receiving data tag */
#define TAG_STANDBY 49 /**< Thoe node standby tag */
#define TAG_RESULT 59 /**< The data result tag */
#define TAG_TERMINATE 32763 /** The node terminate tag */

#define MASTER 0 /**< The master node */
#define NORMAL_MODE 600 /**< The normal operation mode */
#define RESTART_MODE 601 /**< The restart mode */

/* MPI Communication type */
#define MPI_NONBLOCKING 303 /**< Non-blocking communication mode */
#define MPI_BLOCKING 333 /**< Blocking communication mode */

/* Data */
#define TASK_NO_LOCATION -99 /**< Task location defaults */
#define HEADER_SIZE 3+TASK_BOARD_RANK /**< The data header size */
#define HEADER_INIT {TAG_TERMINATE,TASK_EMPTY,TASK_EMPTY,TASK_NO_LOCATION,TASK_NO_LOCATION,TASK_NO_LOCATION}

#define STORAGE_NULL -1

#define STORAGE_END {.name = NULL, .dataspace = H5S_SIMPLE, .datatype = -1, .mpi_datatype = MPI_DOUBLE, .rank = 0, .dims = {0, 0, 0, 0}, .offsets = {0, 0, 0, 0}, .use_hdf = 0, .sync = 0, .storage_type = STORAGE_NULL} /**< The storage scheme default initializer */
#define ATTR_STORAGE_END {.name = NULL, .dataspace = -1, .datatype = -1, .mpi_datatype = MPI_DOUBLE, .rank = 0, .dims = {0, 0, 0, 0}, .offsets = {0, 0, 0, 0}, .use_hdf = 0, .sync = 0, .storage_type = STORAGE_NULL} /**< The attribute storage scheme default initializer */

/**
 * The configuration
 */
#define OPTIONS_END {.space="", .name="", .shortName='\0', .value="", .description="", .type=0} /**< Options default initializer */

#define C_VAL POPT_ARG_VAL /**< int */
#define C_INT POPT_ARG_INT /**< int */
#define C_FLOAT POPT_ARG_FLOAT /**< float */
#define C_DOUBLE POPT_ARG_DOUBLE /**< double */
#define C_STRING POPT_ARG_STRING /**< string */
#define C_LONG POPT_ARG_LONG /**< long */

#define CONFIG_MAX_LINE_LENGTH 1024 /**< Maximum line length in the config file */
#define CONFIG_LEN 512 /**< Maximum config filename length */
#define CONFIG_NULL '\0' /**< Null */

/**
 * @struct config
 * Internal options struct
 */
typedef struct config{
  char name[CONFIG_LEN]; /**< The name of the variable */
  char value[CONFIG_LEN]; /**< The value of the variable */
  int type; /**< The datatype of the variable */
  struct config* next; /**< The next option pointer */
} config;

/**
 * @struct configNamespace
 * The internal namespace struct
 */
typedef struct configNamespace{
  char space[CONFIG_LEN]; /**< The name of the namespace */
  config* options; /**< The options strcuture for given namespace */
  struct configNamespace* next; /**< The pointer to the next namespace */
} configNamespace;

/**
 * @struct options
 * The configuration options
 */
typedef struct {
  char space[CONFIG_LEN]; /**< The name of the configuration namespace */
  char name[CONFIG_LEN]; /**< The name of the variable */
  char shortName; /**< The short name of the variable */
  char value[CONFIG_LEN]; /**< The default value of the variable */
  char description[CONFIG_LEN]; /**< The description of the variable */
  int type; /**< The datatype of the variable */
} options;

/**
 * Configuration helpers
 */

/* Search and modify */
config* ConfigFindOption(char* var, configNamespace* current);
config* ModifyOption(char* space, char* var, char* value, int type, configNamespace* head);

/* Converters */
int Option2Int(char* space, char* var, configNamespace* head);
long Option2Long(char* space, char* var, configNamespace* head);
float Option2Float(char* space, char* var, configNamespace* head);
double Option2Double(char* space, char* var, configNamespace* head);
char* Option2String(char* space, char* var, configNamespace* head);

/**
 * @struct init
 * Bootstrap initializations
 */
typedef struct {
  int options; /**< The maxium size of the Config options table */
  int pools; /**< The maximum size of the pools array */
  int banks_per_pool; /**< The maximum number of memory/storage banks per pool */
  int banks_per_task; /**< The maximum number of memory/storage banks per task */
  int attr_per_dataset; /**< The maximum number of attributes per dataset */
  int min_cpu_required; /**< The minimum number of CPUs required */
} init;

/**
 * @struct popt
 * Popt
 */
typedef struct {
  struct poptOption *popt; /**< The Popt options */
  poptContext poptcontext; /**< The Popt context */
  char **string_args; /**< String arguments received from Popt */
  int *val_args; /**< Value arguments received from Popt */
  int *int_args; /**< Integer arguments received from Popt */
  long *long_args; /**< Long integer arguments received from Popt */
  float *float_args; /**< Float arguments received from Popt */
  double *double_args; /**< Double arguments received from Popt */
} popt;

/**
 * @struct setup
 * The setup structure, combines Config and Popt
 */
typedef struct {
  options *options; /**< The Config default options table */
  configNamespace *head; /**< The Config options linked list */
  popt *popt; /**< The popt options, @see popt */
} setup;

/**
 * @struct schema
 * Defines the memory/storage schema
 */
typedef struct {
  char *name; /**< The name of the dataset */
  int rank; /**< The rank of the dataset */
  int storage_type; /**< The storage type: STORAGE_GROUP, STORAGE_PM3D, STORAGE_BOARD, STORAGE_LIST */
  int use_hdf; /**< Enables HDF5 storage for the memory block */
  int sync; /**< Whether to synchronize memory bank between master and worker */
  int dims[MAX_RANK]; /**< The dimensions of the memory dataset */
  hid_t datatype; /**< The datatype of the dataset */
  int storage_dim[MAX_RANK]; /**< @internal The dimensions of the storage dataset */
  int offsets[MAX_RANK]; /**< @internal The offsets (calculated automatically) */
  H5S_class_t dataspace; /**< @internal The type of the HDF5 dataspace (H5S_SIMPLE) */
  MPI_Datatype mpi_datatype; /**< @internal The MPI datatype of the dataset */
  size_t size; /**< @internal The size of the memory block */
  size_t storage_size; /**< @internal The size of the storage block */
  size_t datatype_size; /** @internal The size of the datatype */
  int elements; /**< @internal Number of data elements in the memory block */
  int storage_elements; /**< @internal Number of data elements in the storage block */
} schema;

/**
 * @struct attr
 * Defines the attribute memory/storage schema
 */
typedef struct {
  schema layout; /**< The memory/storage schema, @see schema */
  char* memory; /**< The memory block */
} attr;

/**
 * @struct storage
 * The storage structure
 */
typedef struct {
  schema layout; /**< The memory/storage schema, @see schema */
  char *memory; /**< The memory block */
  attr *attr; /**< The dataset attributes */
  int attr_banks; /**< Number of attribute banks in use */
} storage;

/**
 * @struct task
 * The task
 */
typedef struct {
  int pid; /**< The parent pool id */
  int tid; /**< The task id */
  int status; /**< The task status */
  int location[TASK_BOARD_RANK]; /**< Coordinates of the task */
  short node; /** The computing node */
  storage *storage; /**< The storage schema and data */
} task;

/**
 * @struct checkpoint
 * The checkpoint
 */
typedef struct {
  int cid; /**< The checkpoint id */
  int counter; /**< The checkpoint internal counter */
  int size; /**< The actual checkpoint size */
  storage *storage; /**< The checkpoint data */
} checkpoint;

/**
 * @struct pool
 * The pool
 */
typedef struct {
  int pid; /**< The pool id */
  int rid; /**< The reset id */
  int status; /**< The pool create status */
  int state; /**< The pool processing state */
  storage *board; /**< The task board */
  storage *storage; /**< The global pool storage scheme */
  task *task; /**< The task scheme */
  task **tasks; /**< All tasks */
  int checkpoint_size; /**< The checkpoint size */
  int pool_size; /**< The pool size (number of tasks to do) */
  int node; /**< The node ID */
  int mpi_size; /**< The MPI COMM size */
} pool;

/**
 * @enum MessageType
 * The types of messages
 */
typedef enum {
  MESSAGE_INFO, /**< The message info type */
  MESSAGE_ERR, /**< The message error type (only the message) */
  MESSAGE_WARN, /**< The warning message type */
  MESSAGE_DEBUG, /**< The debug message type */
  MESSAGE_OUTPUT, /**< The clear output message type */
  MESSAGE_RESULT, /**< The result output message type */
  MESSAGE_COMMENT, /**< The comment message type */
} MessageType;

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

/**
 * High level memory allocation macros
 */

/**
 * @macro
 * Allocate the 2D contiguous array 
 */
#define MAllocate2(_mobject, _mstorage_name, _mbuffer, _mtype)\
  if (_mobject) {\
    int _msindex, _i = 0;\
    int _dim0, _dim1;\
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
    int _msindex, _i = 0, _j = 0;\
    int _dim0, _dim1, _dim2;\
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
    int _msindex, _i = 0, _j = 0, _k = 0;\
    int _dim0, _dim1, _dim2, _dim3;\
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
 * Data read/write helpers
 */
int GetSize(int rank, int *dims); /**< Get the 1D size for given rank and dimensions */
void GetDims(storage *s, int *dims); /**< Get the dimensions of the storage object */
int CopyData(void *in, void *out, size_t size); /**< Copy data buffers */

/**
 * Direct read/write interface
 */
int WriteData(storage *s, void* data); /**< Copy local data buffers to memory (by storage index) */
int ReadData(storage *s, void* data); /**< Copy memory buffers to local data buffers (by storage index) */
int WriteAttr(attr *a, void* data); /**< Copy local attribute buffers to memory (by attribute index) */
int ReadAttr(attr *a, void* data); /**< Copy attribute buffers to local data buffers (by attribute index */

int GetStorageIndex(storage *s, char *storage_name); /**< Get the index for given storage bank */
int GetAttributeIndex(attr *a, char *storage_name); /**< Get the index for given attribute */

int ReadPool(pool *p, char *storage_name, void *data); /**< Read pool data */
int WritePool(pool *p, char *storage_name, void *data); /**< Write data to the pool */

int ReadTask(task *t, char *storage_name, void *data); /**< Read task data */
int WriteTask(task *t, char *storage_name, void *data); /**< Write data to the task */

/**
 * Message and log helpers
 */
void Message(int type, char* message, ...); /**< Common printf wrapper */
int Ice(); /** Checks for the ICE file */
void Error(int status); /**< Error reporting */
void Abort(int status); /**< Abort handler */
void CheckStatus(int status); /**< Status checking utility*/
void H5CheckStatus(hid_t status); /**< HDF5 status checking utility*/

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
    Message(MESSAGE_ERR, "MWriteAttr: Invalid object\n");\
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

#endif

