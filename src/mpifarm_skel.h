#ifndef MPIFARM_SKEL_H
#define MPIFARM_SKEL_H

#include <stdio.h>
#include <stdlib.h>
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
#include "hdf5_hl.h"

#include "mpifarm_user.h"
#include "readconfig.h"

#define DATASETCONFIG "/config"
#define DATABOARD "/board" 
#define DATAGROUP "/data"
#define DATASETMASTER "master"

#define HDF_RANK 2

#define CONFIG_FILE_DEFAULT "config"
#define MODULE_DEFAULT "test"

#undef MY_DATATYPE
#define MY_DATATYPE double

#undef MAX_RESULT_LENGTH
#define MAX_RESULT_LENGTH 12

#undef MY_MPI_DATATYPE
#define MY_MPI_DATATYPE MPI_DOUBLE

/**
 * Master data struct
 */
typedef struct {
 int coords[3]; //0 - x 1 - y 2 - number of the pixel
 MY_DATATYPE res[MAX_RESULT_LENGTH];
} masterData;

#define ITEMS_IN_ARRAY(x) sizeof(x)/sizeof(*(x))

struct yourdata;
extern struct yourdata *makeyourdata(void);

typedef void (*module_init) ();
module_init mpifarm_module_init;

typedef void (*module_query) ();
module_query mpifarm_module_query;

typedef void (*module_cleanup) ();
module_cleanup mpifarm_module_cleanup;

/*extern void mpifarm_module_init(struct yourdata *);
extern void mpifarm_module_cleanup(struct yourdata *);
extern void mpifarm_module_query(struct yourdata *);
*/
/*
struct inputData_t;
extern struct inputData_t *makeInputData(void);
*/
struct slaveData_t;
extern struct slaveData_t *makeSlaveData(void);

/**
 * userdefined functions
 */
extern int userdefined_farmResolution(int, int);
extern void userdefined_pixelCompute(int, inputData_t *d, masterData *r, struct slaveData_t *s);
extern void userdefined_pixelCoords(int, int t[], inputData_t *d, masterData *r, struct slaveData_t *s);
extern void userdefined_pixelCoordsMap(int ind[], int, int, int);
extern void userdefined_masterIN(int, inputData_t *d);
extern void userdefined_masterOUT(int, inputData_t *d, masterData *r);
extern void userdefined_slaveIN(int, inputData_t *d, masterData *r, struct slaveData_t *s);
extern void userdefined_slaveOUT(int, inputData_t *d, masterData *r, struct slaveData_t *s);
extern void userdefined_master_beforeSend(int, inputData_t *d, masterData *r);
extern void userdefined_master_afterSend(int, inputData_t *d, masterData *r);
extern void userdefined_master_beforeReceive(inputData_t *d, masterData *r);
extern void userdefined_master_afterReceive(int, inputData_t *d, masterData *r);
extern void userdefined_slave_beforeSend(int, inputData_t *d, masterData *r, struct slaveData_t *s);
extern void userdefined_slave_afterSend(int, inputData_t *d,  masterData *r, struct slaveData_t *s);
extern void userdefined_slave_beforeReceive(int, inputData_t *d, masterData *r, struct slaveData_t *s);
extern void userdefined_slave_afterReceive(int, inputData_t *d, masterData *r, struct slaveData_t *s);

extern int userdefined_readConfigValues(char*, char*, char*, inputData_t *d);
extern void userdefined_mpiBcast(int, inputData_t *d);

configOptions options[MAX_OPTIONS_NUM];
configNamespace configSpace[MAX_CONFIG_SIZE];

#endif
