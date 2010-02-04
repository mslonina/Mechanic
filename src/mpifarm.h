#ifndef MPIFARM_H
#define MPIFARM_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/dir.h>
#include <dirent.h>
#include <math.h>
#include <popt.h>
#include <dlfcn.h>

#include "mpi.h"
#include "hdf5.h"

#include "libreadconfig.h"

#define MECHANIC_VERSION "UNSTABLE-2"
#define MECHANIC_AUTHOR "MSlonina, TCfA, NCU"
#define MECHANIC_EMAIL "mariusz.slonina@gmail.com"

#define MECHANIC_CONFIG_FILE_DEFAULT "config"
#define MECHANIC_NAME_DEFAULT "showme"
#define MECHANIC_MODULE_DEFAULT "default"
#define MECHANIC_MASTER_FILE_DEFAULT "default-master.h5"
#define MECHANIC_XRES_DEFAULT 5
#define MECHANIC_YRES_DEFAULT 5
#define MECHANIC_METHOD_DEFAULT 0
#define MECHANIC_MRL_DEFAULT 10
#define MECHANIC_DUMP_DEFAULT 2000
#define MECHANIC_FILE 1024
#define MECHANIC_FILE_OLD 1028

#define MECHANIC_MODULE_SILENT 0
#define MECHANIC_MODULE_WARN 1
#define MECHANIC_MODULE_ERROR 2

#define MECHANIC_MPI_DEST 0
#define MECHANIC_MPI_SOURCE_TAG 0
#define MECHANIC_MPI_DATA_TAG 2
#define MECHANIC_MPI_RESULT_TAG 59
#define MECHANIC_MPI_TERMINATE_TAG 99

#define MECHANIC_DATASETCONFIG "/config"
#define MECHANIC_DATABOARD "/board" 
#define MECHANIC_DATAGROUP "/data"
#define MECHANIC_DATASETMASTER "master"

#define MECHANIC_ERR_MPI 911
#define MECHANIC_ERR_HDF 912
#define MECHANIC_ERR_MODULE 913
#define MECHANIC_ERR_SETUP 914
#define MECHANIC_ERR_OTHER 999

#define MECHANIC_HDF_RANK 2

#undef MECHANIC_DATATYPE
#define MECHANIC_DATATYPE double

#undef MECHANIC_MPI_DATATYPE
#define MECHANIC_MPI_DATATYPE MPI_DOUBLE

#define ITEMS_IN_ARRAY(x) sizeof(x)/sizeof(*(x))

/**
 * MASTER DATA
 */
typedef struct {
 int coords[3]; //0 - x 1 - y 2 - number of the pixel
 MECHANIC_DATATYPE res[1];
} masterData;

/**
 * MODULE ARCHITECTURE FUNCTION HANDLERS
 */
typedef void (*module_init_f) ();
module_init_f init;

typedef void (*module_query_void_f) ();
module_query_void_f query;

typedef int (*module_query_int_f) ();
module_query_int_f iquery;

typedef void (*module_cleanup_f) ();
module_cleanup_f cleanup;

/* MAIN CONFIG DATA */
typedef struct {
  char name[256];
  char datafile[260];
  char module[256];
  int xres;
  int yres;
  int method;
  int mrl;
  int checkpoint;
  int restartmode;
} configData;

/* Module info and handler */
typedef struct {
      const char *name;
      const char *author;
      const char *date;
      const char *version;
} moduleInfo;

#endif
