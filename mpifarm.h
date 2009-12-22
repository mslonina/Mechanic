#ifndef MPIFARM_H
#define MPIFARM_H

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

#include "mpi.h"
#include "hdf5.h"
#include "hdf5_hl.h"

#include "mpifarm_user.h"

#define DATASETCONFIG "/config"
#define DATASETMAP "/board" 
#define DATASETRAW "/data"

#define HDF_RANK 2

typedef struct {
 int coords[3]; //0 - x 1 - y 2 - number of the pixel
 MY_DATATYPE res[MAX_RESULT_LENGTH];
} outputData;

#define ITEMS_IN_ARRAY(x) sizeof(x)/sizeof(*(x))

/**
 * userdefined functions
 */
extern int userdefined_farmResolution(int, int);
extern void userdefined_pixelCompute(int, outputData *r);
extern void userdefined_pixelCoords(int, int t[],outputData *r);
extern void userdefined_pixelCoordsMap(int ind[], int, int, int);
extern void userdefined_masterIN();
extern void userdefined_masterOUT();
extern void userdefined_slaveIN(int);
extern void userdefined_slaveOUT(int);
extern void userdefined_master_beforeSend(int, outputData *r);
extern void userdefined_master_afterSend(int, outputData *r);
extern void userdefined_master_beforeReceive(outputData *r);
extern void userdefined_master_afterReceive(int, outputData *r);
extern void userdefined_slave_beforeSend(int, outputData *r);
extern void userdefined_slave_afterSend(int, outputData *r);
extern void userdefined_slave_beforeReceive(int, outputData *r);
extern void userdefined_slave_afterReceive(int, outputData *r);

#endif
