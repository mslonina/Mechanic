/**
 * @file
 * The Mechanic core headers
 */
#ifndef MECHANIC_H
#define MECHANIC_H 1

#include "libreadconfig.h"
#include "libreadconfig_hdf5.h"
#include "hdf5.h"

#define MECHANIC_MAX_HDF_RANK 7

#define MECHANIC_ERR_MPI 911
#define MECHANIC_ERR_HDF 912
#define MECHANIC_ERR_MODULE 913
#define MECHANIC_ERR_SETUP 914
#define MECHANIC_ERR_MEM 915
#define MECHANIC_ERR_CHECKPOINT 916
#define MECHANIC_ERR_OTHER 999
#define MECHANIC_ICE 31337
#define MECHANIC_TASK_SUCCESS 0

#define MECHANIC_MODULE_ERR_MPI 811
#define MECHANIC_MODULE_ERR_HDF 812
#define MECHANIC_MODULE_ERR_MODULE 813
#define MECHANIC_MODULE_ERR_SETUP 814
#define MECHANIC_MODULE_ERR_MEM 815
#define MECHANIC_MODULE_ERR_CHECKPOINT 816
#define MECHANIC_MODULE_ERR_OTHER 888

#define MECHANIC_HDF_RANK 2
#define MECHANIC_STRLEN 256

#undef MECHANIC_DATATYPE
#define MECHANIC_DATATYPE double

#undef MECHANIC_MPI_DATATYPE
#define MECHANIC_MPI_DATATYPE MPI_DOUBLE

#define ITEMS_IN_ARRAY(x) sizeof(x)/sizeof(*(x))

enum Messages{
  MECHANIC_MESSAGE_INFO,
  MECHANIC_MESSAGE_ERR,
  MECHANIC_MESSAGE_IERR,
  MECHANIC_MESSAGE_CONT,
  MECHANIC_MESSAGE_CONT2,
  MECHANIC_MESSAGE_WARN,
	MECHANIC_MESSAGE_DEBUG
} mechanicMessages;

/* [CONFIGDATA] */
typedef struct {
  char name[MECHANIC_STRLEN];
  char datafile[MECHANIC_STRLEN];
  char module[MECHANIC_STRLEN];
  char mconfig[MECHANIC_STRLEN];
  int name_len; /* Fortran requirement: length of the name string */
  int datafile_len; /* Fortran requirement: length of the datafile string */
  int module_len; /* Fortran requirement: length of the module string */
  int mconfig_len; /* Fortran requirement: length of the module string */
  /*unsigned long*/ int xres;
  /*unsigned long*/ int yres;
  /*unsigned long*/ int checkpoint;
  /*unsigned short*/ int restartmode;
  /*unsigned short*/ int mode;
} TaskConfig;
/* [/CONFIGDATA] */

/* [MASTERDATA] */
typedef struct {
  MECHANIC_DATATYPE *data;
  /*unsigned long*/ int coords[3]; /* 0 - x 1 - y 2 - number of the pixel */
} TaskData;
/* [/MASTERDATA] */

/* [SCHEMA] */
typedef struct {
  char *path;
  H5S_class_t type;
  hid_t datatype;
  unsigned int rank;
  hsize_t dimsize[MECHANIC_MAX_HDF_RANK]; /* Fortran max dims */
} mechanicSchema;
/* [/SCHEMA] */

/* [MODULEINFO] */
typedef struct {
  int input_length;
  int output_length;
  int api;
  int schemasize;
  int options;
  mechanicSchema *schema;
  LRC_configDefaults *mconfig;
  LRC_configNamespace *moptions;
} TaskInfo;

/* [/MODULEINFO] */


/* MECHANIC INTERNALS STRUCT */
typedef struct {
  int mpi_size;
  int node;
  int sendnode;
  int recvnode;
  int nodes;
  int comm;
  void* module;
  void* handler;
  TaskConfig* config;
  TaskInfo* info;
  char* ice;
} mechanic_internals;

typedef struct {
  int api;
  char name[MECHANIC_STRLEN];
  char datafile[MECHANIC_STRLEN];
  char module[MECHANIC_STRLEN];
  int name_len; /* Fortran requirement: length of the name string */
  int datafile_len; /* Fortran requirement: length of the datafile string */
  int module_len; /* Fortran requirement: length of the module string */
  int schemasize;
  int input_length;
  int output_length;
  struct {
    int comm;
    int mpisize;
    int node;
    int nodes;
    int sendnode;
    int recvnode;
  } mpi;
  struct {
    char *path;
    H5S_class_t type;
    hid_t datatype;
    unsigned int rank;
    hsize_t dimsize[MECHANIC_MAX_HDF_RANK];
  } schema;
  struct {
    /*unsigned long*/ int xres;
    /*unsigned long*/ int yres;
    /*unsigned long*/ int checkpoint;
    /*unsigned short*/ int restartmode;
    /*unsigned short*/ int mode;
  } config;
} mechanic_info_t;

typedef struct {
  void *handler;
  void *module;
  mechanic_info_t info;
} mechanic_internals_t;

void mechanic_message(int type, char* fmt, ...);
int mechanic_finalize(/*unsigned long*/ int node);
int mechanic_abort(int errcode);
void mechanic_error(int stat);
void mechanic_check_mstat(int stat);

#endif

