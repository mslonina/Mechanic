#ifndef MECHANIC_INTERNALS_H
#define MECHANIC_INTERNALS_H

#if HAVE_CONFIG_H
  #include <config.h>
#endif

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

/* GLOBALS */
char* inifile;
char* datafile;
int allopts, mpi_rank, mpi_size;
int usage, help;

/* FUNCTION PROTOTYPES */
int* map2d(int, void* handler, moduleInfo*, configData* d);
void master(void* handler, moduleInfo*, configData* d, int restartmode);
void slave(void* handler, moduleInfo*, configData* d);
void clearArray(MECHANIC_DATATYPE*,int);
void buildMasterResultsType(int mrl, masterData* md, MPI_Datatype* masterResultsType_ptr);
void buildDefaultConfigType(configData* d, MPI_Datatype* defaultConfigType_ptr);
void* load_sym(void* handler, moduleInfo*, char* function, int type);
int readDefaultConfig(char* inifile, LRC_configNamespace* cs, LRC_configTypes* ct, int numCT, int flag);
void assignConfigValues(int opts, configData* d, LRC_configNamespace* cs, int flag, int popt);
void H5writeMaster(hid_t dset, hid_t memspace, hid_t space, configData* d, int* coordsarr, MECHANIC_DATATYPE* resultarr);
void H5writeBoard(hid_t dset, hid_t memspace, hid_t space, int* coordsarr);
void mechanic_displayArgs(poptContext con, enum poptCallbackReason reason, const struct poptOption* key, 
    char* arg, void* data);
void mechanic_displayUsage(poptContext con, enum poptCallbackReason reason, const struct poptOption* key, 
    char* arg, void* data);
void poptTestC(char* i, char* j);
void poptTestI(char* i, int j);
void welcome();
void H5createMasterDataScheme(hid_t file_id, configData* d);
void H5writeCheckPoint(configData* d, int check, int** coordsarr, MECHANIC_DATATYPE** resultarr);
void H5readBoard(configData* d, int** board);

#define MECHANIC_POPT_AUTOHELP { NULL, '\0', POPT_ARG_INCLUDE_TABLE, mechanic_poptHelpOptions, \
			0, "Help options:", NULL },


#endif
