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

#include "readconfig.h"

#define CONFIG_FILE_DEFAULT "config"
#define NAME_DEFAULT "showme"
#define MODULE_DEFAULT "default"
#define MASTER_FILE_DEFAULT "default-master.h5"

#define DATASETCONFIG "/config"
#define DATABOARD "/board" 
#define DATAGROUP "/data"
#define DATASETMASTER "master"

#define HDF_RANK 2

#undef MY_DATATYPE
#define MY_DATATYPE double

#undef MY_MPI_DATATYPE
#define MY_MPI_DATATYPE MPI_DOUBLE

/**
 * Master data struct
 */
typedef struct {
 int coords[3]; //0 - x 1 - y 2 - number of the pixel
 MY_DATATYPE res[1];
} masterData;

#define ITEMS_IN_ARRAY(x) sizeof(x)/sizeof(*(x))

struct yourdata;
extern struct yourdata *makeyourdata(void);

/**
 * Plugin architecture function handlers
 */
typedef void (*module_init_f) ();
module_init_f init;

typedef void (*module_query_void_f) ();
module_query_void_f query;

typedef int (*module_query_int_f) ();
module_query_int_f iquery;

typedef void (*module_cleanup_f) ();
module_cleanup_f cleanup;

/**
 * Plugin architecture datatypes prototypes
 */
/*
struct inputData_t;
extern struct inputData_t *makeInputData(void);
*/
struct slaveData_t;
extern struct slaveData_t *makeSlaveData(void);

configOptions options[MAX_OPTIONS_NUM];
configNamespace configSpace[MAX_CONFIG_SIZE];

/* Main config file struct */
typedef struct {
  char name[256];
  char datafile[260];
  char module[256];
  int xres;
  int yres;
  int method;
  int mrl;
  int dump;
} configData;

/* Simplified struct for writing config data to hdf file */
typedef struct {
      char space[MAX_NAME_LENGTH];
      char varname[MAX_NAME_LENGTH];
      char value[MAX_VALUE_LENGTH];
    } simpleopts;

char* sep = "="; char* comm = "#";
char* inifile;
char* datafile;

#endif
