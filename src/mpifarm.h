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

#define VERSION "UNSTABLE-2"
#define AUTHOR "MSlonina, TCfA, NCU"
#define EMAIL "mariusz.slonina@gmail.com"

#define CONFIG_FILE_DEFAULT "config"
#define NAME_DEFAULT "showme"
#define MODULE_DEFAULT "default"
#define MASTER_FILE_DEFAULT "default-master.h5"
#define XRES_DEFAULT 5
#define YRES_DEFAULT 5
#define METHOD_DEFAULT 0
#define MRL_DEFAULT 10
#define DUMP_DEFAULT 2000

#define MODULE_SILENT 0
#define MODULE_WARN 1
#define MODULE_ERROR 2

#define MPI_DEST 0
#define MPI_SOURCE_TAG 0
#define MPI_DATA_TAG 2
#define MPI_RESULT_TAG 59
#define MPI_TERMINATE_TAG 99

#define DATASETCONFIG "/config"
#define DATABOARD "/board" 
#define DATAGROUP "/data"
#define DATASETMASTER "master"

#define ERR_MPI 911
#define ERR_HDF 912
#define ERR_MODULE 913
#define ERR_SETUP 914
#define ERR_OTHER 999

#define HDF_RANK 2

#undef MY_DATATYPE
#define MY_DATATYPE double

#undef MY_MPI_DATATYPE
#define MY_MPI_DATATYPE MPI_DOUBLE

#define ITEMS_IN_ARRAY(x) sizeof(x)/sizeof(*(x))

/**
 * MASTER DATA
 */
typedef struct {
 int coords[3]; //0 - x 1 - y 2 - number of the pixel
 MY_DATATYPE res[1];
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

/* Simplified struct for writing config data to hdf file */
typedef struct {
      char space[MAX_NAME_LENGTH];
      char varname[MAX_NAME_LENGTH];
      char value[MAX_VALUE_LENGTH];
    } simpleopts;

/* Module info and handler */
typedef struct {
      const char *name;
      const char *author;
      const char *date;
      const char *version;
} moduleInfo;

#endif
