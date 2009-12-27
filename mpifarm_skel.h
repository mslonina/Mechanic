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

#include "mpi.h"
#include "hdf5.h"
#include "hdf5_hl.h"

#include "mpifarm_user.h"
#include "libreadconfig.h"

#define DATASETCONFIG "/config"
#define DATABOARD "/board" 
#define DATAGROUP "/data"
#define DATASETMASTER "master"

#define HDF_RANK 2

/**
 * Master data struct
 */
typedef struct {
 int coords[3]; //0 - x 1 - y 2 - number of the pixel
 MY_DATATYPE res[MAX_RESULT_LENGTH];
} masterData;

#define ITEMS_IN_ARRAY(x) sizeof(x)/sizeof(*(x))

/**
 * userdefined functions
 */
extern int userdefined_farmResolution(int, int);
extern void userdefined_pixelCompute(int, inputData *d, masterData *r, slaveData *s);
extern void userdefined_pixelCoords(int, int t[], inputData *d, masterData *r, slaveData *s);
extern void userdefined_pixelCoordsMap(int ind[], int, int, int);
extern void userdefined_masterIN(int, inputData *d);
extern void userdefined_masterOUT(int, inputData *d, masterData *r);
extern void userdefined_slaveIN(int, inputData *d, masterData *r, slaveData *s);
extern void userdefined_slaveOUT(int, inputData *d, masterData *r, slaveData *s);
extern void userdefined_master_beforeSend(int, inputData *d, masterData *r);
extern void userdefined_master_afterSend(int, inputData *d, masterData *r);
extern void userdefined_master_beforeReceive(inputData *d, masterData *r);
extern void userdefined_master_afterReceive(int, inputData *d, masterData *r);
extern void userdefined_slave_beforeSend(int, inputData *d, masterData *r, slaveData *s);
extern void userdefined_slave_afterSend(int, inputData *d,  masterData *r, slaveData *s);
extern void userdefined_slave_beforeReceive(int, inputData *d, masterData *r, slaveData *s);
extern void userdefined_slave_afterReceive(int, inputData *d, masterData *r, slaveData *s);

extern int userdefined_readConfigValues(char*, char*, char*, inputData *d);
extern void userdefined_mpiBcast(int, inputData *d);

configOptions options[MAX_OPTIONS_NUM];
configNamespace configSpace[MAX_CONFIG_SIZE];

#endif
