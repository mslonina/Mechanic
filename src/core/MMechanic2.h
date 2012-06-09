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

#include <mpi.h>
#include <hdf5.h>
#include <libreadconfig.h>
#include <libreadconfig_hdf5.h>

#define TASK_SUCCESS 0
#define CORE_ICE 112
#define CORE_SETUP_HELP 212
#define CORE_SETUP_USAGE 213

/* Error codes */
#define CORE_ERR_CORE 901
#define CORE_ERR_MPI 911
#define CORE_ERR_HDF 912
#define CORE_ERR_MODULE 913
#define CORE_ERR_SETUP 914
#define CORE_ERR_MEM 915
#define CORE_ERR_CHECKPOINT 916
#define CORE_ERR_STORAGE 917
#define CORE_ERR_OTHER 999

#define MODULE_ERR_MPI 811
#define MODULE_ERR_HDF 812
#define MODULE_ERR_MODULE 813
#define MODULE_ERR_SETUP 814
#define MODULE_ERR_MEM 815
#define MODULE_ERR_CHECKPOINT 816
#define MODULE_ERR_OTHER 888

/* Pool */
#define POOL_FINALIZE 1001
#define POOL_RESET 1002
#define POOL_CREATE_NEW 1003

/* Storage */
#define MAX_RANK 2
#define STORAGE_BASIC 11
#define STORAGE_PM3D 12
#define STORAGE_BOARD 13
#define STORAGE_LIST 14

/* Task */
#define TASK_EMPTY -88
#define TASK_FINISHED 1
#define TASK_AVAILABLE 0
#define TASK_IN_USE -1
#define NO_MORE_TASKS -99

/**
 * @struct
 * Bootstrap initializations
 */
typedef struct {
  int options; /**< The maxium size of the LRC options table */
  int pools; /**< The maximum size of the pools array */
  int banks_per_pool; /**< The maximum number of memory/storage banks per pool */
  int banks_per_task; /**< The maximum number of memory/storage banks per task */
} init;

/**
 * @struct
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
 * @struct
 * The setup structure, combines LRC and Popt
 */
typedef struct {
  LRC_configDefaults *options; /**< The LRC default options table */
  LRC_configNamespace *head; /**< The LRC options linked list */
  popt *popt; /**< The popt options, @see popt */
} setup;

/**
 * @struct
 * Defines the memory/storage schema
 */
typedef struct {
  char *path; /**< The name of the dataset */
  int rank; /**< The rank of the dataset */
  int dim[MAX_RANK]; /**< The dimensions of the dataset */
  int offset[MAX_RANK]; /**< The offsets (calculated automatically) */
  int use_hdf; /**< Enables HDF5 storage for the memory block */
  int sync; /**< @unused*/
  int storage_type; /**< The storage type: STORAGE_BASIC, STORAGE_PM3D, STORAGE_BOARD, STORAGE_LIST */
  H5S_class_t dataspace_type; /**< The type of the HDF5 dataspace (H5S_SIMPLE) */
  hid_t datatype; /**< The datatype of the dataset (H5T_NATIVE_DOUBLE) */
} schema;

/**
 * @struct
 * The storage structure
 */
typedef struct {
  schema layout; /**< The memory/storage schema, @see schema */
  double **data; /**< The data pointer */
} storage;

/**
 * @struct
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
 * @struct
 * The checkpoint
 */
typedef struct {
  int cid; /**< The checkpoint id */
  int counter; /**< The checkpoint internal counter */
  int size; /**< The actual checkpoint size */
  storage *storage; /**< The checkpoint data */
} checkpoint;

/**
 * @struct
 * The pool
 */
typedef struct {
  int pid; /**< The pool id */
  int rid; /**< The reset id */
  storage *board; /**< The task board */
  storage *storage; /**< The global pool storage scheme */
  task *task; /**< The task scheme */
  task *tasks; /**< All tasks */
  int checkpoint_size; /**< The checkpoint size multiplier */
  int pool_size; /**< The pool size (number of tasks to do) */
  int node; /**< The node ID */
  int mpi_size; /**< The MPI COMM size */
} pool;

enum {
  MESSAGE_INFO,
  MESSAGE_ERR,
  MESSAGE_IERR,
  MESSAGE_CONT,
  MESSAGE_CONT2,
  MESSAGE_WARN,
	MESSAGE_DEBUG
} MessageType;

void Message(int type, char* message, ...);

#define MASTER 0
#define NORMAL_MODE 600
#define RESTART_MODE 601

#define STORAGE_END {.path = NULL, .dataspace_type = H5S_SIMPLE, .datatype = H5T_NATIVE_DOUBLE, .rank = 0, .dim = {0, 0}, .use_hdf = 0, .sync = 0, .storage_type = -1}

#endif
