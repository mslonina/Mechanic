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
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/param.h>
#include <signal.h>
#include <fcntl.h>
#include <dirent.h>
#include <math.h>
#include <popt.h>
#include <dlfcn.h>
#include <time.h>

#include <mpi.h>
#include <hdf5.h>
#include <hdf5_hl.h>
#include <libreadconfig.h>
#include <libreadconfig_hdf5.h>

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

/* Storage */
#define MAX_RANK 2 /**< The maximum dataset rank */
#define STORAGE_GROUP 11 /**< The basic data storage type */
#define STORAGE_PM3D 12 /**< The pm3d data storage type */
#define STORAGE_BOARD 13 /**< The board data storage type */
#define STORAGE_LIST 14 /**< The list data storage type */

/* Task */
#define TASK_EMPTY -88 /**< The task empty return code */
#define TASK_FINISHED 1 /**< The task finished return code */
#define TASK_AVAILABLE 0 /**< The task available return code */
#define TASK_IN_USE -1 /**< The task in use return code */
#define NO_MORE_TASKS -99 /**< No more tasks return code */

#define MASTER 0 /**< The master node */
#define NORMAL_MODE 600 /**< The normal operation mode */
#define RESTART_MODE 601 /**< The restart mode */

/* MPI Communication type */
#define MPI_NONBLOCKING 303 /**< Non-blocking communication mode */
#define MPI_BLOCKING 333 /**< Blocking communication mode */

/* Data */
#define TASK_NO_LOCATION -1
#define HEADER_SIZE 3+MAX_RANK /**< The data header size */
#define HEADER_INIT {TASK_EMPTY,TASK_EMPTY,TASK_EMPTY,TASK_NO_LOCATION,TASK_NO_LOCATION}

#define STORAGE_END {.path = NULL, .dataspace_type = H5S_SIMPLE, .datatype = -1, .mpi_datatype = MPI_DOUBLE, .rank = 0, .dim = {0, 0}, .offset ={0, 0}, .use_hdf = 0, .sync = 0, .storage_type = -1} /**< The storage scheme default initializer */

/**
 * @struct init
 * Bootstrap initializations
 */
typedef struct {
  int options; /**< The maxium size of the LRC options table */
  int pools; /**< The maximum size of the pools array */
  int banks_per_pool; /**< The maximum number of memory/storage banks per pool */
  int banks_per_task; /**< The maximum number of memory/storage banks per task */
  int attr_per_dataset; /**< The maximum number of attributes per dataset */
} init;

/**
 * @struct popt
 * Popt
 */
typedef struct {
  struct poptOption *popt; /**< The Popt options */
  poptContext poptcontext; /**< The Popt context */
  char **string_args; /**< String arguments received from Popt */
  int *int_args; /**< Integer arguments received from Popt */
  double *double_args; /**< Double arguments received from Popt */
} popt;

/**
 * @struct setup
 * The setup structure, combines LRC and Popt
 */
typedef struct {
  LRC_configDefaults *options; /**< The LRC default options table */
  LRC_configNamespace *head; /**< The LRC options linked list */
  popt *popt; /**< The popt options, @see popt */
} setup;

/**
 * @struct schema
 * Defines the memory/storage schema
 */
typedef struct {
  char *path; /**< The name of the dataset */
  int rank; /**< The rank of the dataset */
  int dim[MAX_RANK]; /**< The dimensions of the dataset */
  int offset[MAX_RANK]; /**< The offsets (calculated automatically) */
  int use_hdf; /**< Enables HDF5 storage for the memory block */
  int sync; /**< Whether to synchronize memory bank between master and worker */
  int storage_type; /**< The storage type: STORAGE_GROUP, STORAGE_PM3D, STORAGE_BOARD, STORAGE_LIST */
  H5S_class_t dataspace_type; /**< The type of the HDF5 dataspace (H5S_SIMPLE) */
  hid_t datatype; /**< The datatype of the dataset (H5T_NATIVE_DOUBLE) */
  MPI_Datatype mpi_datatype; /**< The MPI datatype of the dataset */
  size_t size; /**< The size of the memory block */
  size_t datatype_size; /** The size of the datatype */
  int elements; /**< Number of data elements in the memory block */
} schema;

typedef struct {
  schema layout;
  double **data;
  char* memory;
} attr;

/**
 * @struct storage
 * The storage structure
 */
typedef struct {
  schema layout; /**< The memory/storage schema, @see schema */
  char *memory; /**< The memory block */
  double **data; /**< The data pointer */
  attr *attr; /**< The dataset attributes */
} storage;

/**
 * @struct task
 * The task
 */
typedef struct {
  int pid; /**< The parent pool id */
  int tid; /**< The task id */
  int status; /**< The task status */
  int location[MAX_RANK]; /**< Coordinates of the task */
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

void Message(int type, char* message, ...);
void PrintDataset(int type, hid_t dataset);
double** AllocateBuffer(int rank, int *dims);
void FreeBuffer(double **array);
int GetSize(int rank, int *dims);

int Allocate(storage *s, size_t size, size_t datatype); /**< Memory allocator */
void Free(storage *s); /**< Garbage cleaner */
int WriteData(storage *s, void* data); /**< Copy local data buffers to memory */
int ReadData(storage *s, void* data); /**< Copy memory buffers to local data buffers */

#endif
