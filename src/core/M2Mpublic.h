/**
 * @file
 * MPI-communication (public API)
 */
#ifndef MECHANIC_M2M_PUBLIC_H
#define MECHANIC_M2M_PUBLIC_H

#include "M2Apublic.h"

#include <mpi.h>
#include <hdf5.h>

/* Message tags */
#define TAG_DATA 1337 /**< Send/Receiving data tag */
#define TAG_STANDBY 49 /**< The node standby tag */
#define TAG_RESULT 59 /**< The data result tag */
#define TAG_CHECKPOINT 69 /**< The data checkpoint tag */
#define TAG_TERMINATE 32763 /** The node terminate tag */

#define MASTER 0 /**< The master node */
#define DEST 0
#define SOURCE 0

/* MPI Communication type */
#define MPI_NONBLOCKING 303 /**< Non-blocking communication mode */
#define MPI_BLOCKING 333 /**< Blocking communication mode */

#endif
