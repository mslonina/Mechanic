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
#include <ltdl.h>

#include <mpi.h>
#include <hdf5.h>
#include <libreadconfig.h>
#include <libreadconfig_hdf5.h>

/* Error codes */
#define TASK_SUCCESS 0
#define CORE_ERR_CORE 901
#define CORE_ERR_MPI 911
#define CORE_ERR_HDF 912
#define CORE_ERR_MODULE 913
#define CORE_ERR_SETUP 914
#define CORE_ERR_MEM 915
#define CORE_ERR_CHECKPOINT 916
#define CORE_ERR_OTHER 999
#define CORE_ICE 31337

#define MODULE_ERR_MPI 811
#define MODULE_ERR_HDF 812
#define MODULE_ERR_MODULE 813
#define MODULE_ERR_SETUP 814
#define MODULE_ERR_MEM 815
#define MODULE_ERR_CHECKPOINT 816
#define MODULE_ERR_OTHER 888

#define POOL_FINALIZE 0
#define POOL_CREATE_NEW 1

#define MAX_RANK 2

typedef struct {
  int options;
  int pools;
  int banks_per_pool;
  int banks_per_task;
} init;

typedef struct {
  LRC_configDefaults *options;
  LRC_configNamespace *head;
  struct poptOption *popt;
  poptContext poptcontext;
} setup;

typedef struct {
  char *path;
  int use_hdf;
  int rank;
  int storage_type;
  H5S_class_t dataspace_type;
  hid_t datatype;
  int dim[MAX_RANK];
} schema;

typedef struct {
  schema layout;
  double **data;
} storage;

typedef struct {
  int pid; /* The parent pool id */
  int tid; /* The task id */
  int location[MAX_RANK]; /* Coordinates of the task */
  storage *storage;
} task;

typedef struct {
  int pid; /* The pool id */
  char name[LRC_CONFIG_LEN]; /* The pool name */
  char *path; /* */
  hid_t location;
  storage *storage;
  task task;
} pool;


#endif
