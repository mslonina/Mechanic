#ifndef MECHANIC_MECHANIC_H
#define MECHANIC_MECHANIC_H

#include <inttypes.h> /* for 32 and 64 bits */
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
#define POOL_CREATE_NEW 1003

/* Storage */
#define MAX_RANK 2
#define STORAGE_BASIC 11
#define STORAGE_PM3D 12
#define STORAGE_BOARD 13
#define STORAGE_SINGLE 14
#define STORAGE_LIST 15

/* Task */
#define TASK_EMPTY -88
#define TASK_FINISHED 1
#define TASK_AVAILABLE 0
#define TASK_IN_USE -1
#define NO_MORE_TASKS -99

typedef struct {
  int options;
  int pools;
  int banks_per_pool;
  int banks_per_task;
} init;

typedef struct {
  struct poptOption *popt;
  poptContext poptcontext;
  char **string_args;
  int *int_args;
  double *double_args;
} popt;

typedef struct {
  LRC_configDefaults *options;
  LRC_configNamespace *head;
  popt *popt;
} setup;

typedef struct {
  char *path;
  int rank;
  int dim[MAX_RANK];
  int use_hdf;
  int sync;
  int storage_type;
  H5S_class_t dataspace_type;
  hid_t datatype;
} schema;

typedef struct {
  schema layout;
  double **data;
} storage;

typedef struct {
  int pid; /* The parent pool id */
  int tid; /* The task id */
  int status;
  char* path;
  int location[MAX_RANK]; /* Coordinates of the task */
  storage *storage;
} task;

typedef struct {
  int cid; /* The checkpoint id */
  int counter;
  int size;
  storage *storage;
  double **data;
} checkpoint;

typedef struct {
  int pid; /* The pool id */
  char *path; /* */
  char name[LRC_CONFIG_LEN]; /* The pool name */
  storage *board;
  storage *storage;
  task *task;
  int checkpoint_size;
  int pool_size;
  int node;
  int mpi_size;
} pool;

#define STORAGE_END {.path = NULL, .dataspace_type = H5S_SIMPLE, .datatype = H5T_NATIVE_DOUBLE, .rank = 0, .dim = {0, 0}, .use_hdf = 0, .sync = 0, .storage_type = STORAGE_BASIC}

#endif
