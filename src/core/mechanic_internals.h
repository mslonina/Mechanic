#ifndef MECHANIC_INTERNALS_H
#define MECHANIC_INTERNALS_H 1

#if HAVE_CONFIG_H
  #include <config.h>
#endif

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

#include "mpi.h"
#include "hdf5.h"
#include "libreadconfig.h"
#include "libreadconfig_hdf5.h"

#if PLATFORM_DARWIN == 1
  #define WE_ARE_ON_DARWIN 1
  #define LIB_EXT ".dylib"
  #define LIB_ESZ 6
#endif

#if PLATFORM_LINUX == 1
  #define WE_ARE_ON_LINUX 1
  #define LIB_EXT ".so"
  #define LIB_ESZ 3
#endif

#define MECHANIC_NAME PACKAGE_NAME
#define MECHANIC_VERSION PACKAGE_VERSION
#define MECHANIC_VERSION_MAJOR PACKAGE_VERSION_MAJOR
#define MECHANIC_VERSION_MINOR PACKAGE_VERSION_MINOR
#define MECHANIC_VERSION_PATCH PACKAGE_VERSION_PATCH
#define MECHANIC_AUTHOR PACKAGE_AUTHOR
#define MECHANIC_BUGREPORT PACKAGE_BUGREPORT
#define MECHANIC_URL PACKAGE_URL

#define MECHANIC_CONFIG_FILE_DEFAULT "config"
#define MECHANIC_CONFIG_GROUP "mechanic"
#define MECHANIC_NAME_DEFAULT "mechanic"
#define MECHANIC_MODULE_DEFAULT "module"
#define MECHANIC_MASTER_PREFIX_DEFAULT "master"
#define MECHANIC_FILE_EXT "h5"
#define MECHANIC_MASTER_FILE_DEFAULT "module-master.h5"
#define MECHANIC_MASTER_SUFFIX_DEFAULT "-master-00.h5"
#define MECHANIC_XRES_DEFAULT "5"
#define MECHANIC_YRES_DEFAULT "5"
#define MECHANIC_METHOD_DEFAULT "0"
#define MECHANIC_CHECKPOINT_FILE_DEFAULT "mechanic-master-00.h5"
#define MECHANIC_CHECKPOINT_DEFAULT "2000"
#define MECHANIC_CHECKPOINTS 3
#define MECHANIC_MODE_DEFAULT "1"
#define MECHANIC_MRL_DEFAULT 4
#define MECHANIC_IRL_DEFAULT 4
#define MECHANIC_MODULE_PREFIX "libmechanic_module_"

#define MECHANIC_FILE 256
#define MECHANIC_FILE_OLD_PREFIX "backup-"
#define MECHANIC_MAXLENGTH MAXPATHLEN

#define MECHANIC_MODULE_SILENT 0
#define MECHANIC_MODULE_WARN 1
#define MECHANIC_MODULE_ERROR 2

#define MECHANIC_MPI_DEST 0
#define MECHANIC_MPI_SOURCE_TAG 0
#define MECHANIC_MPI_DATA_TAG 2
#define MECHANIC_MPI_STANDBY_TAG 49
#define MECHANIC_MPI_RESULT_TAG 59
#define MECHANIC_MPI_TERMINATE_TAG 99

#define MECHANIC_DATASETCONFIG "/config"
#define MECHANIC_DATABOARD "/board"
#define MECHANIC_DATAGROUP "/data"
#define MECHANIC_STATSGROUP "/stats"
#define MECHANIC_DATASETMASTER "master"

#define MECHANIC_STORAGE_API PACKAGE_VERSION_API
#define MECHANIC_MODULE_API 1
#define MECHANIC_MODULE_API_C "1"
#define MECHANIC_MPI_MASTER_NODE 0

#define MECHANIC_TASK_FINISHED 1
#define MECHANIC_TASK_NOTSTARTED 0
#define MECHANIC_TASK_STARTED -1

#define MECHANIC_TEMPLATE 1
#define MECHANIC_NO_TEMPLATE 0

enum Modes {
  MECHANIC_MODE_MASTERALONE,
  MECHANIC_MODE_FARM,
} mechanicModes;

/* POPT MODES */
#define MECHANIC_MODE_MASTERALONE_P '0'
#define MECHANIC_MODE_FARM_P '1'

/* MODULE ARCHITECTURE FUNCTION HANDLERS */
typedef int (*module_query_void_f) ();
typedef int (*module_query_int_f) ();

/* LRC-MPI */
typedef struct {
  char space[LRC_CONFIG_LEN];
  char name[LRC_CONFIG_LEN];
  char value[LRC_CONFIG_LEN];
  int type;
} LRC_MPIStruct;

LRC_MPIStruct* allocateLRCMPIStruct(int options);
int LRC2MPI(LRC_MPIStruct*, LRC_configNamespace* head);
char* mechanic_module_filename(char* name);
mechanic_internals* mechanic_module_open(char* module);
void mechanic_module_close(mechanic_internals* module);
mechanic_internals* mechanic_internals_init(int mpi_size, int node, TaskInfo* m, TaskConfig* d);
void mechanic_internals_schema_init(int node, TaskInfo* m, mechanic_internals* internals);
void mechanic_internals_close(mechanic_internals* handler);

/* GLOBALS */
char* TaskConfigFile;
char* TaskInfoTaskConfigFile;
char* datafile;
char* CheckpointFile;
int allopts;
int usage, help, debug, silent;

/* FUNCTION PROTOTYPES */
int map2d(int c, mechanic_internals *handler, int ind[], int** b);

int buildMasterResultsType(int mrl, TaskData* md,
    MPI_Datatype* masterResultsType_ptr);
int buildTaskConfigDataType(int lengths[4], TaskConfig d,
    MPI_Datatype* TaskConfigType_ptr);
int LRC_datatype(LRC_MPIStruct cc, MPI_Datatype* lrc_mpi_t);

char* mechanic_module_sym_prefix(char* prefix, char* function);
module_query_int_f mechanic_load_sym(mechanic_internals *handler, char* function, int type, int tp);
module_query_int_f mechanic_sym_lookup(void* modhand, char* md_name, char* function);

int readDefaultTaskConfig(char* inifile, int flag, LRC_configNamespace* head);
int readCheckpointTaskConfig(char* inifile, char* group, LRC_configNamespace* head);

int assignTaskConfigValues(TaskConfig* d, LRC_configNamespace* head);

int H5writeMaster(hid_t dset, hid_t memspace, hid_t space, TaskInfo *md,
    TaskConfig* d, int* coordsarr, MECHANIC_DATATYPE* resultarr);

int H5writeBoard(hid_t dset, hid_t memspace, hid_t space, int* coordsarr);

int H5createMasterDataScheme(hid_t file_id, TaskInfo *md, TaskConfig* d);
int H5createMasterAttributes(hid_t loc_id);

int manageCheckPoints(TaskConfig *d);

int H5readBoard(TaskConfig* d, int** board, int *computed);

int H5writeCheckPoint(TaskInfo *md, TaskConfig* d, int check,
    int** coordsarr, MECHANIC_DATATYPE** resultarr);

int atCheckPoint(mechanic_internals *handler, int check, int** coordsarr, int** board,
    MECHANIC_DATATYPE** resultarr);

int mechanic_validate_file(char* filename);

int mechanic_ups();
int prepare_ice(mechanic_internals* internals);
int mechanic_ice(mechanic_internals* handler);

void mechanic_welcome();

void clearArray(MECHANIC_DATATYPE*,int);

int mechanic_mode_multifarm(mechanic_internals* handler);
int mechanic_mode_farm(mechanic_internals* handler);
int mechanic_mode_masteralone(mechanic_internals* handler);

int validate_arg(char* arg);
int mechanic_printTaskConfig(TaskConfig* cd, int flag);
int mechanic_copy(char* in, char* out);

int* AllocateIntVec(int x);
double* AllocateDoubleVec(int x);

int** AllocateInt2D(int x, int y);
double** AllocateDouble2D(int x, int y);

int* IntArrayToVec(int** array, int x, int y);
double* DoubleArrayToVec(double** array, int x, int y);

void FreeIntVec(int* vec);
void FreeInt2D(int** pointer, int elems);
void FreeDoubleVec(double* vec);
void FreeDouble2D(double** pointer, int elems);

#define MECHANIC_POPT_AUTOHELP { NULL, '\0', POPT_ARG_INCLUDE_TABLE,\
  mechanic_poptHelpOptions, 0, "Help options:", NULL },

#define MECHANIC_POPT_MODES { NULL, '\0', POPT_ARG_INCLUDE_TABLE,\
  mechanic_poptModes, 0, "Modes:", NULL },

#define MECHANIC_POPT_RESTART { NULL, '\0', POPT_ARG_INCLUDE_TABLE,\
  mechanic_poptRestart, 0, "Restart options:", NULL },

#define MECHANIC_POPT_DEBUG { NULL, '\0', POPT_ARG_INCLUDE_TABLE,\
  mechanic_poptDebug, 0, "Debug/Message interface options:", NULL },

#endif

