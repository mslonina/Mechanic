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

#if HAVE_MPI_SUPPORT
  #include "mpi.h"
#endif

#include "hdf5.h"

#include "libreadconfig.h"

#define MECHANIC_VERSION "0.12 UNSTABLE-2"
#define MECHANIC_AUTHOR "MSlonina, TCfA, NCU"
#define MECHANIC_EMAIL "mariusz.slonina@gmail.com"

#define MECHANIC_CONFIG_FILE_DEFAULT "config"
#define MECHANIC_NAME_DEFAULT "showme"
#define MECHANIC_MODULE_DEFAULT "echo"
#define MECHANIC_MASTER_FILE_DEFAULT "echo-master.h5"
#define MECHANIC_XRES_DEFAULT 5
#define MECHANIC_YRES_DEFAULT 5
#define MECHANIC_METHOD_DEFAULT 0
#define MECHANIC_CHECKPOINT_DEFAULT 2000
#define MECHANIC_CHECKPOINTS 6
#define MECHANIC_CHECKPOINT_NUM_DEFAULT 0

#if HAVE_MPI_SUPPORT
  #define MECHANIC_MODE_DEFAULT 1
#else
  #define MECHANIC_MODE_DEFAULT 0
#endif

#define MECHANIC_FILE 256
#define MECHANIC_FILE_OLD 260
#define MECHANIC_PATH MAXPATHLEN

#define MECHANIC_MODULE_SILENT 0
#define MECHANIC_MODULE_WARN 1
#define MECHANIC_MODULE_ERROR 2

#if HAVE_MPI_SUPPORT
  #define MECHANIC_MPI_DEST 0
  #define MECHANIC_MPI_SOURCE_TAG 0
  #define MECHANIC_MPI_DATA_TAG 2
  #define MECHANIC_MPI_RESULT_TAG 59
  #define MECHANIC_MPI_TERMINATE_TAG 99
#endif

#define MECHANIC_DATASETCONFIG "/config"
#define MECHANIC_DATABOARD "/board" 
#define MECHANIC_DATAGROUP "/data"
#define MECHANIC_DATASETMASTER "master"

enum Modes {
  MECHANIC_MASTERALONE, 
#if HAVE_MPI_SUPPORT
  MECHANIC_FARM, 
  MECHANIC_GRID, 
  MECHANIC_MULTIFARM
#endif
} mechanicModes;

/**
 * MODULE ARCHITECTURE FUNCTION HANDLERS
 */
typedef int (*module_init_f) ();
module_init_f init;

typedef int (*module_query_void_f) ();
module_query_void_f query;

typedef int (*module_query_int_f) ();
module_query_int_f iquery;

typedef int (*module_cleanup_f) ();
module_cleanup_f cleanup;

/* GLOBALS */
char* inifile;
char* datafile;
int allopts, mpi_rank, mpi_size;
int usage, help;

/* FUNCTION PROTOTYPES */
int* map2d(int, void* handler, moduleInfo*, configData* d);

#if HAVE_MPI_SUPPORT
int buildMasterResultsType(int mrl, masterData* md, MPI_Datatype* masterResultsType_ptr);
int buildDefaultConfigType(configData* d, MPI_Datatype* defaultConfigType_ptr);
#endif

void* load_sym(void* handler, moduleInfo*, char* function, int type);
int readDefaultConfig(char* inifile, LRC_configNamespace* cs, LRC_configTypes* ct, int numCT, int flag);
int assignConfigValues(int opts, configData* d, LRC_configNamespace* cs, int flag, int popt);
void mechanic_displayArgs(poptContext con, enum poptCallbackReason reason, const struct poptOption* key, 
    char* arg, void* data);
void mechanic_displayUsage(poptContext con, enum poptCallbackReason reason, const struct poptOption* key, 
    char* arg, void* data);
void poptTestC(char* i, char* j);
void poptTestI(char* i, int j);

int H5writeMaster(hid_t dset, hid_t memspace, hid_t space, moduleInfo *md, configData* d, int* coordsarr, MECHANIC_DATATYPE* resultarr);
int H5writeBoard(hid_t dset, hid_t memspace, hid_t space, int* coordsarr);
int H5createMasterDataScheme(hid_t file_id, moduleInfo *md, configData* d);

int manageCheckpoints(configData *d);
int H5readBoard(configData* d, int** board);
int H5writeCheckPoint(moduleInfo *md, configData* d, int check, int** coordsarr, MECHANIC_DATATYPE** resultarr);
int atCheckPoint(int check, int** coordsarr, int** board, MECHANIC_DATATYPE** resultarr, moduleInfo *md, configData* d);

void welcome();
void clearArray(MECHANIC_DATATYPE*,int);
int mechanic_finalize(int node);
int mechanic_abort(int errcode);

#define MECHANIC_POPT_AUTOHELP { NULL, '\0', POPT_ARG_INCLUDE_TABLE, mechanic_poptHelpOptions, \
			0, "Help options:", NULL },

#define MECHANIC_POPT_MODES { NULL, '\0', POPT_ARG_INCLUDE_TABLE, mechanic_poptModes, \
			0, "Modes:", NULL },

#define MECHANIC_POPT_RESTART { NULL, '\0', POPT_ARG_INCLUDE_TABLE, mechanic_poptRestart, \
			0, "Restart options:", NULL },
#endif
