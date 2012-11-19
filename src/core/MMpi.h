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

int LRC_datatype(options c, MPI_Datatype *mpi_t);

int Pack(module *m, void *buffer, pool *p, task *t, int tag);
int Unpack(module *m, void *buffer, pool *p, task *t, int *tag);

#endif
