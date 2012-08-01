/**
 * @file
 * MPI-related includes and prototypes
 */
#ifndef MECHANIC_MPI_H
#define MECHANIC_MPI_H

#include "Mechanic2.h"
#include "MCommon.h"
#include "MPool.h"
#include "MTask.h"
#include "MLog.h"

#define DEST 0
#define SOURCE 0
#define TAG_DATA -1337
#define TAG_STANDBY 49
#define TAG_RESULT 59
#define TAG_TERMINATE -32763

int LRC_datatype(LRC_configDefaults c, MPI_Datatype *mpi_t);

int Pack(module *m, double *buffer, pool *p, task *t, int tag);
int Unpack(module *m, double *buffer, pool *p, task *t, int *tag);

#endif
