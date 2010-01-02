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

//typedef struct (*slaveData_f) ();
//slaveData_f qs;

/**
 * Module architecture datatypes prototypes
 */
/*
struct inputData_t;
extern struct inputData_t *makeInputData(void);
*/
//yourData yd;
//extern yourData *makeyourdata(void);
//struct slaveData;
//struct slaveData *s;
//extern struct slaveData *makeSlaveData(void);

configOptions options[MAX_OPTIONS_NUM];
configNamespace configSpace[MAX_CONFIG_SIZE];

/* MAIN CONFIG DATA */
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

/* GLOBALS */
char* inifile;
char* datafile;
int allopts, mpi_rank, mpi_size;

/* FUNCTION PROTOTYPES */
int* map2d(int, void*, configData* d);
void master(void*, configData* d);
void slave(void*, configData* d);
void clearArray(MY_DATATYPE*,int);
void buildMasterResultsType(int mrl, masterData* md, MPI_Datatype* masterResultsType_ptr);
void buildDefaultConfigType(configData* d, MPI_Datatype* defaultConfigType_ptr);
int readDefaultConfig(char* inifile, configData* cd);
void* load_sym(void* module, char* function, int status);
void writeConfig(hid_t file_id, int allopts);
void H5writeMaster(hid_t dset, hid_t memspace, hid_t space, configData* d, masterData* rawdata);
void H5writeBoard(hid_t dset, hid_t memspace, hid_t space, masterData* rawdata);

#endif
