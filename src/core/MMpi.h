/**
 * @file
 * MPI-related includes and prototypes
 */
#ifndef MECHANIC_MPI_H
#define MECHANIC_MPI_H

#include "MMechanic2.h"

#define DEST 0
#define SOURCE 0
#define TAG_DATA 2
#define TAG_STANDBY 49
#define TAG_RESULT 59
#define TAG_TERMINATE -32763

int LRC_datatype(LRC_configDefaults c, MPI_Datatype *mpi_t);

#endif
