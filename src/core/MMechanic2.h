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

typedef struct {
  int options;
  int pools;
} init;

typedef struct {
  LRC_configDefaults *options;
  LRC_configNamespace *head;
} setup;

typedef struct {
} task;

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

#endif
